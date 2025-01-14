#pragma once

#include <filesystem>
#include <mutex>
#include <optional>

#include "handlers/api_handler.h"
#include "handlers/log_request_handler.h"
#include "handlers/db_use_cases.h"

namespace http_handler {

std::vector<std::string> ReadURL(const http::request<http::string_body>& req);
std::string ReadStrURL(const http::request<http::string_body>& req);
http::response<http::string_body> FileNotFound(
        const http::request<http::string_body>& req);

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit RequestHandler(net::io_context& ioc, model::Game& game, 
                            const std::filesystem::path& path_to_content, 
                            const std::filesystem::path& path_to_state,
                            size_t period, bool random_spawn, size_t save_interval,
                            json_loot::LootTypes&& loot_types, 
                            postgres::DataBase& data_base)
        : strand_{net::make_strand(ioc)},
          api_handler_{game, strand_, period, random_spawn, save_interval, 
                       std::move(loot_types), path_to_state, data_base},
          path_to_content_{path_to_content} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, 
                    Send&& send) {
        std::string str = ReadStrURL(req);
        if (str.find("api") != std::string::npos) {
                      // TODO Handle usind Strand
                      // after postgreSQL
            mutex.lock();
            send(std::move(api_handler_.MakeResponse(req, std::move(str))));
            mutex.unlock();
        } else {
            std::vector<std::string> parseURL{std::move(ReadURL(req))};
            std::optional<http::response<http::file_body>> response = 
                MakeFileResponse(req, parseURL);
            if (response) {
                send(std::move(*response));
            } else {
                send(std::move(FileNotFound(req)));
            }   
        }
    }

std::optional<http::response<http::file_body>> MakeFileResponse(
        const http::request<http::string_body> &req,
        const std::vector<std::string>& URL_path);     
void SaveState();                                        

private:
    Strand strand_;
    std::mutex mutex;
    ApiHandler api_handler_;
    const std::filesystem::path path_to_content_;
};

std::filesystem::path MakePath (const std::vector<std::string>& path,
                                std::filesystem::path path_to_content_);
std::string DecodeURL(const std::string& target_url);
bool IsInRoot(const std::filesystem::path& root, 
              const std::filesystem::path& resource);
std::string MimeType(std::string& expand);
void DownScale(std::string& string);
}  // namespace http_handler
