#include "TODO/web/TODO.hpp"
#include <charconv>
#include <optional>
#include <string_view>

const std::string WEB_FILES_PATH = "files";

//#####################################################################
//                         HTTP CODE
//#####################################################################

enum class HTTP_Code
{
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,

    NOT_FOUND = 404,

    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501, 
};

std::string to_string(HTTP_Code ec)
{
    switch (ec)
    {
    case HTTP_Code::OK:
        return "OK";

    case HTTP_Code::CREATED:
        return "CREATED";

    case HTTP_Code::NO_CONTENT:
        return "NO_CONTENT";
    
    case HTTP_Code::NOT_FOUND:
        return "NOT_FOUND";
    
    case HTTP_Code::INTERNAL_SERVER_ERROR:
        return "INTERNAL_SERVER_ERROR";
    
    case HTTP_Code::NOT_IMPLEMENTED:
        return "NOT_IMPLEMENTED";
    
    default:
        return "NULL";
    }
}

//#####################################################################
//                         HTTP METHOD
//#####################################################################

enum class HTTP_Method
{
    NONE = 0,
    HEAD,

    GET,
    POST,
    PUT,
    DELETE,


    PATCH,
};
typedef HTTP_Method Method;

std::string to_string(Method m)
{
    switch (m)
    {
    case Method::HEAD:
        return "HEAD";
    
    case Method::GET:
        return "GET";
   
    case Method::POST:
        return "POST";
    
    case Method::PUT:
        return "PUT";
    
    case Method::PATCH:
        return "PATCH";
   
    case Method::DELETE:
        return "DELETE";
 
    case Method::NONE:
    default:
        return "NULL";
    }
}

//#####################################################################
//                          HTTP MESSAGE 
//#####################################################################

struct HTTP_Message
{
    std::string _header = {};
    size_t _body_len = 0;

    Method _method = Method::NONE;

    std::string _body = {};

    bool check_len() { return _body.size() == _body_len; }

    void clear()
    {
        _header.clear();
        _body.clear();

        _body_len = 0;
        _method = Method::NONE;
    }
};

//#####################################################################
//                         HELPER FUNCTIONS 
//#####################################################################

std::tuple<std::string, HTTP_Code> read_file(std::string file_name)
{
    std::cout << "file name: " << file_name << "\n";
    std::ifstream file(file_name, std::ios::binary);
    if(!file.is_open())
        return {"", HTTP_Code::NOT_FOUND };


    std::ostringstream ss;
    ss << file.rdbuf();

    return { ss.str(), HTTP_Code::OK };
}

size_t find_nth(std::string_view str, size_t n, std::string_view find) 
{
    if ( n == 0 ) { return std::string::npos; }
    size_t pos, from = 0;

    for (size_t i=0; i<n; ++i) 
    {
        pos=str.find(find, from);
        if ( std::string::npos == pos ) { break; }
        from = pos + 1;
    }
    return pos;
}

//#####################################################################
//                         WEB SERVER
//#####################################################################

class WebServer 
{
private:
    asio::strand<asio::any_io_executor> _strand;
    TCP_Server _base_server;
    ProjectArchive _proj_arch;
    //std::string _request;
    HTTP_Message _http_request;
    std::function<asio::awaitable<void>(std::string*, std::string *)> _route_func;
    
    //TODO: abstract this method to call based on url example: TODO/...; Foo/...
    asio::awaitable<void> call_todo_func(std::string *content, std::string *query);
    
    
    asio::awaitable<bool> handle_request(std::string *request, std::string *response);
    bool is_request_ready(std::string *request);
    std::string url_decode(const std::string& input);

public:
    explicit WebServer(asio::any_io_executor executor);
    //void start(/* port */);
    void start(asio::io_context &context);
};

WebServer::WebServer(asio::any_io_executor executor)
    : _strand(asio::make_strand(executor)),
      _route_func(nullptr),
      _base_server(
          [this]
          (
              std::string* request,
              std::string* response
          ) -> asio::awaitable<bool>
          {
              co_return co_await handle_request(request, response);
          }
      )
{
}

std::string WebServer::url_decode(const std::string& input)
{
    std::string output;

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '%' &&
            i + 2 < input.size() &&
            std::isxdigit(input[i + 1]) &&
            std::isxdigit(input[i + 2]))
        {
            std::string hex = input.substr(i + 1, 2);

            char c = static_cast<char>(
                std::stoi(hex, nullptr, 16)
            );

            output += c;
            i += 2;
        }
        else if (input[i] == '+')
        {
            // application/x-www-form-urlencoded
            output += ' ';
        }
        else
        {
            output += input[i];
        }
    }

    return output;
}


asio::awaitable<void> WebServer::call_todo_func(std::string *content, std::string *query)
{
    std::cout << "Calling " << to_string(_http_request._method) << "\n";

    std::function<void(std::string*, std::string*, ProjectArchive*)> func;
    if(_http_request._method == Method::GET)
    {
        func = show_projects;
    }
    else if(_http_request._method == Method::PUT)
    {
        func = update_projects;
    }
    else if(_http_request._method == Method::POST)
    {
        func = add_element;
    }
    else if(_http_request._method == Method::DELETE)
    {
        func = delete_element;
    }

    co_await asio::co_spawn(
        _strand,
        [this, func, content, query]() -> asio::awaitable<void>
        {
            func(content, query, &_proj_arch);
            co_return;
        },
        asio::use_awaitable
    ); 
}

bool WebServer::is_request_ready(std::string* request)
{
    std::cout << "Total request size: "
              << request->size()
              << " bytes\n";

    const auto header_end = request->find("\r\n\r\n");

    if (header_end == std::string::npos)
    {
        std::cout << "Headers incomplete -> request NOT ready\n";
        return false;
    }

    const size_t headers_size = header_end + 4;
    const size_t new_body_size = request->size() - headers_size;

    std::cout << "Headers size: "
              << headers_size
              << " bytes\n";

    std::cout << "New Body size: "
              << new_body_size
              << " bytes\n";

    const std::string headers = request->substr(0, header_end);

    std::cout << "Headers:\n"
              << headers
              << "\n";

    const std::string body_new = request->substr(headers_size);

    std::cout << "New Body:\n"
              << body_new
              << "\n";
    
    
    const std::string body = _http_request._body + body_new;
    std::cout << "Full Body:\n"
              << body
              << "\n";

    const size_t body_size = body.size();

    const auto content_length_pos = headers.find("Content-Length:");

    _http_request._header = std::move(headers);
    if (content_length_pos == std::string::npos)
    {
        std::cout << "No Content-Length -> request READY\n";
        return true;
    }

    auto value_start =
        content_length_pos + std::string("Content-Length:").size();

    while (value_start < headers.size() &&
           std::isspace(
               static_cast<unsigned char>(headers[value_start])))
    {
        ++value_start;
    }

    const auto value_end = headers.find("\r\n", value_start);

    if (value_end == std::string::npos)
    {
        std::cout << "Invalid Content-Length header -> request NOT ready\n";
        return false;
    }

    const std::string content_length_str =
        headers.substr(
            value_start,
            value_end - value_start
        );

    size_t content_length = 0;

    try
    {
        content_length = std::stoull(content_length_str);
    }
    catch (const std::exception& e)
    {
        std::cout << "Invalid Content-Length value: "
                  << content_length_str
                  << "\n";

        return false;
    }

    //update request
    //if request is complete, body will contain all sub-parts, if not it will contain 0 or more, same process for both
    _http_request._body = std::move(body);
    _http_request._body_len = content_length;

    std::cout << "Content-Length: "
              << content_length
              << " bytes\n";
    

    if (body_size < content_length)
    {
        std::cout << "Missing body bytes: "
                  << content_length - body_size
                  << "\n";

        std::cout << "Complete request: NO\n";

        return false;
    }

    std::cout << "Complete request: YES\n";

    return true;
}

asio::awaitable<bool> WebServer::handle_request(std::string *request, std::string *response)
{
    if(!_http_request._header.empty())
    {
        _http_request._body += *request;
        if(!_http_request.check_len())
        {
            std::cout << "request was not ready\n";
            co_return false;
        }
    }
    else
    {
        if(!is_request_ready(request))
        {
            std::cout << "request was not ready\n";
            co_return false;
        }
    }

    std::cout << "Request header: \n" << _http_request._header << "\n";
    std::cout << "Request body: " << _http_request._body << "\n";

    std::stringstream sstream(_http_request._header.data());

    std::string start_line = {};
    getline(sstream, start_line);

    _http_request._method = Method::NONE;
    
    if(start_line.starts_with("GET"))
    {
        _http_request._method = Method::GET;
    }
    else if(start_line.starts_with("POST"))
    {
        _http_request._method = Method::POST;
    }
    else if(start_line.starts_with("PUT"))
    {
        _http_request._method = Method::PUT;
    }
    else if(start_line.starts_with("DELETE"))
    {
        _http_request._method = Method::DELETE;
    }
    
    //if none method is unimplemented - return with message   
    if(_http_request._method == Method::NONE)
    {
        HTTP_Code code = HTTP_Code::NOT_IMPLEMENTED;

        std::cout << "Method is none, request: " << _http_request._header << "\n";
        *response = "HTTP/1.1 " + std::to_string(static_cast<int>(code)) + " " + to_string(code) + "\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
        
        co_return true;
    }

    size_t cursor = to_string(_http_request._method).size() + 1;

    // size_t _ = start_line.find(' ');
    // size_t space = start_line.find(' ', _ + 1);
    size_t space = find_nth(start_line, 2, " ");
    std::string web_path = start_line.substr(cursor, space - cursor);

    std::cout << "Web path: " << web_path << "\n";

    size_t route_pos = find_nth(web_path, 2, "/");
    bool has_query = web_path.find('?') != std::string::npos;

    std::string file_name = {};
    std::string query = {};

    if (route_pos != std::string_view::npos && has_query) 
    {
        file_name = web_path.substr(0, route_pos);
        query = std::string(web_path.substr(route_pos + 1));
        query = url_decode(query);
    } 
    else 
    {
        file_name = web_path;
    }

    std::cout << "\nFile name: " << file_name << " | Query: " << query << "\n";

    if (!query.empty()) 
    {
        file_name += "/index.html";
    }

    if (file_name.starts_with("/TODO"))
    {
        //if(file_name.contains("/index.html"))
        _route_func = [this](std::string *content, std::string *query) -> asio::awaitable<void>
        {
            co_await call_todo_func(content, query);
            co_return;
        };
    }
    else
    {
        _route_func = nullptr;
    }
    

    std::string real_path = WEB_FILES_PATH + file_name;

    std::cout << "HTTP Method: " << to_string(_http_request._method) << '\n';
    std::cout << "Real path: |" << real_path << "|\n";

    HTTP_Code code = {};
    if (_http_request._method == Method::GET)
    {
        std::string contents = {};
        std::tie(contents, code) = read_file(real_path);

        if(code != HTTP_Code::OK)
        {
            std::string str_err = to_string(code);

            *response = "HTTP/1.1 " + std::to_string(static_cast<int>(code)) + " " + str_err + "\r\n"
                "Content-Length: 0\r\n"
                "\r\n";

            std::cout << "code: " << str_err << "\n";
        }
        else
        {

            std::cout << "real_path: " << real_path << "\n";
            
            //text/css text/csv text/html text/javascript (obsolete) text/plain text/xml
            std::string content_type = {};
            bool no_html = true;
            if(real_path.contains(".html"))
            {
                no_html = false;
                content_type = "Content-Type: text/html; charset=utf-8\r\n";
            }
            else if(real_path.contains(".css"))
            {
                content_type = "Content-Type: text/css; charset=utf-8\r\n";
            }
            else if(real_path.contains(".js"))
            {
                content_type = "Content-Type: application/javascript; charset=utf-8\r\n";
            }

            if(_route_func && !no_html)
            {
                co_await _route_func(&contents, &query);
            }

            size_t len = contents.size();
            
            *response =  "HTTP/1.1 200 OK\r\n"
                "Content-Length: " + std::to_string(len) + "\r\n"
                + content_type + 
                "Connection: close\r\n"
                "\r\n" 
                + contents;
        }
    }
    if( _http_request._method == Method::PUT    || 
        _http_request._method == Method::POST   ||
        _http_request._method == Method::DELETE )
    {
        code = HTTP_Code::NOT_IMPLEMENTED;
        std::string contents = _http_request._body;

        if(_route_func)
        {
            if(_http_request._method == Method::POST)
            {
                code = HTTP_Code::CREATED;
            }
            else
            {
                //TODO: later if possible change this to ok and provide content
                code = HTTP_Code::NO_CONTENT;
            }

            co_await _route_func(&contents, &query);
        }
        
        //TODO: return modified/added content as string
        *response = "HTTP/1.1 " + std::to_string(static_cast<int>(code)) + " " + to_string(code) + "\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
    }

    _http_request.clear();

    co_return true;
}

void WebServer::start(asio::io_context &context)
{
    _base_server.start(context);
}

int main()
{   
    asio::io_context io;

    WebServer server(io.get_executor());
    server.start(io);
    
    return 0;
}