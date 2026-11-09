#include "../../server_tcp/server.hpp"
#include "../lib/TODO.hpp"
#include "../../DOM_Generator/DOM_Generator.hpp"
#include "../../JSON/JSON_Parser.hpp"
#include <cerrno>
#include <iostream>
#include <filesystem>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <unordered_map>

#define PROJECTS_PATH "files/TODO/project.json"

// Element;typedef std::variant<Project, Objective, Task> Element
using ElementPtr = std::variant<
    Project*,
    Objective*,
    Task*
>;

struct ProjectArchive
{
    std::vector<Project> projects;
    std::unordered_map<long, ElementPtr> elements;
};

std::string to_lower(std::string data); 

void read_projects_json(std::string_view file_name, ProjectArchive *proj_arch);
void save_projects_json(std::string_view file_name, ProjectArchive *proj_arch);

void show_projects(std::string *msg, std::string *query, ProjectArchive *proj_arch);
void update_projects(std::string *msg, std::string *query, ProjectArchive *proj_arch);
void add_element(std::string *msg, std::string *query, ProjectArchive *proj_arch);
void delete_element(std::string *msg, std::string *query, ProjectArchive *proj_arch);