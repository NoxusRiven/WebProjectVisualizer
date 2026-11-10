#include "TODO.hpp"
#include <algorithm>

//#####################################################################
//                          HELPER FUNCTIONS
//#####################################################################

std::string to_lower(std::string data) 
{
    std::transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return data;
}

void read_projects_json(std::string_view file_name, ProjectArchive* proj_arch)
{
    proj_arch->projects.clear();
    proj_arch->elements.clear();

    JSON::JSON_Parser parser;
    std::cout << "calling read_json\n";
    std::cout << "file_name: " << file_name << "\n";
    JSON::JsonValue json = parser.parse(file_name);
    
    //if json is null, file not found, create one
    if(json.is<JSON::j_null>())
    {
        std::cout << "Json file is null\n";
        std::ofstream file(file_name.data());
        if(!file.is_open())
        {
            std::cout << "Cound't create file '" << file_name << "'\n";
            exit(-1);
        }

        file << "{}\n";
        file.flush();
        file.close();

        //call parse again
        json = parser.parse(file_name);
        if(json.is<JSON::j_null>())
        {
            std::cout << "Unreachable! created file and json still was null\n";
        } 
    }

    if(!json.get_as<JSON::j_object>().contains("projects"))
    {
        std::cout << "Empty json file\n";
        return;
    }

    JSON::JsonValue json_projects = json["projects"];

    auto& projects = proj_arch->projects;

    for (auto& proj : json_projects.get_as<JSON::j_array>())
    {
        Project p(
            proj["id"].get_as<double>(),
            proj["name"].get_as<std::string>()
        );

        if (proj.get_as<JSON::j_object>().contains("objectives"))
        {
            for (auto& obj : proj["objectives"].get_as<JSON::j_array>())
            {
                Objective objective(
                    obj["id"].get_as<double>(),
                    obj["name"].get_as<std::string>(),
                    static_cast<Priority>(
                        obj["priority"].get_as<double>()
                    )
                );

                // Tasks
                for (auto& task : obj["tasks"].get_as<JSON::j_array>())
                {
                    Task t(
                        task["id"].get_as<double>(),
                        task["description"].get_as<std::string>()
                    );

                    if (task["finished"].get_as<bool>())
                        t.mark_finished();
                    else
                        t.mark_unfinished();

                    objective.tasks.push_back(std::move(t));
                }

                p.objectives.push_back(std::move(objective));
            }
        }

        projects.push_back(std::move(p));
    }

    for (auto& p : projects)
    {
        proj_arch->elements.try_emplace(p.id, &p);

        for (auto& obj : p.objectives)
        {
            proj_arch->elements.try_emplace(obj.id, &obj);

            for (auto& t : obj.tasks)
                proj_arch->elements.try_emplace(t.id, &t);
        }
    }
}


void save_projects_json(std::string_view file_name, ProjectArchive *proj_arch)
{
    auto *projects = &proj_arch->projects;
    
    sort_projects(projects);
    
    std::ofstream json_file(file_name.data());

    json_file << "{\n";
    json_file << "\"projects\": [\n";

    for(size_t p = 0; p < projects->size(); ++p)
    {
        auto& proj = (*projects)[p];

        json_file << "{\n";
        json_file << "\"id\": " << proj.id << ",\n";
        json_file << "\"name\": \"" << proj.name << "\",\n";
        json_file << "\"objectives\": [\n";

        for(size_t o = 0; o < proj.objectives.size(); ++o)
        {
            auto& obj = proj.objectives[o];

            json_file << "{\n";
            json_file << "\"id\": " << obj.id << ",\n";
            json_file << "\"name\": \"" << obj.name << "\",\n";
            json_file << "\"priority\": " << (int)obj.priority << ",\n";

            // ---------- tasks ----------
            json_file << "\"tasks\": [\n";

            for(size_t t = 0; t < obj.tasks.size(); ++t)
            {
                auto& task = obj.tasks[t];

                json_file << "{\n";
                json_file << "\"id\": " << task.id << ",\n";
                printf("task id %ld\n", task.id);
                json_file << "\"finished\": " << (task.finished ? "true" : "false") << ",\n";
                printf("task finished %d\n", task.finished);
                json_file << "\"description\": \"" << task.description << "\"\n";
                printf("task desc %s\n", task.description.c_str());
                json_file << "}";

                if(t + 1 != obj.tasks.size())
                    json_file << ",";

                json_file << "\n";
            }

            json_file << "]\n";
            json_file << "}";

            if(o + 1 != proj.objectives.size())
                json_file << ",";

            json_file << "\n";
        }

        json_file << "]\n";
        json_file << "}";

        if(p + 1 != projects->size())
            json_file << ",";

        json_file << "\n";
    }

    json_file << "]\n";
    json_file << "}\n";
}

bool objective_compare(Objective a, Objective b)
{
    return a.priority > b.priority;
}

void sort_projects(std::vector<Project> *projects)
{
    auto& projects_tmp = *projects;

    for(auto& proj : projects_tmp)
    {
        std::sort(proj.objectives.begin(), proj.objectives.end(), objective_compare);
    }
}

//#####################################################################
//                         CORE FUNCTION 
//#####################################################################

void show_projects(std::string *msg, std::string *query, ProjectArchive *proj_arch)
{
    auto& projects = proj_arch->projects;
    read_projects_json(PROJECTS_PATH, proj_arch);

    IdHandler id_h(&projects);

    //TODO: also set clicked link to style example: class=clicked
    if(!query->empty()) 
    {
        //split web path by '?' to extract file name and querry
        size_t pos_eq = query->find('=');
        
        *query = query->substr(pos_eq + 1);

        std::string to_find = "{{BODY}}";

        auto base_body = DomElement("");

        bool found = false;
        Project* current_project;
        for(auto& proj : projects)
        {
            if(proj.name == *query)
            {
                found = true;
                current_project = &proj;
            }
        }

        if(found)
        {

            for(auto& obj : current_project->objectives)
            {
                base_body.append(
                    DomElement("div").set_attr({{"class", "objective"}, {"data-id", std::to_string(obj.id)}})
                        .append(DomElement("div").set_attr( {{"class",  "priority " + to_lower(to_string(obj.priority))}} ))
                        .append(
                            DomElement("p").set_attr({{"class", "title"}})
                            .append(obj.name)
                        )
                );
                
                for(auto& t : obj.tasks)
                {
                    Attribute checked;
                    
                    if(t.finished)
                        checked = Attribute("checked");
                    
                    base_body.append(
                        DomElement("div").set_attr({{"class", "task"}, {"data-id", std::to_string(t.id)}})
                            .append(DomElement("input").set_attr({{"type", "checkbox"}, checked}))
                            .append(
                                DomElement("p")
                                    .append(t.description)
                            )
                    );
                }

                base_body
                    .append(
                        DomElement("button").set_attr({ {"class", "task-indent"},
                                                        {"data-type", "task"},
                                                        {"data-parent", std::to_string(obj.id)},
                                                        {"onclick", "addElement(this)"}})
                            .append("Add task")
                    );
            }

            base_body
                .append(
                    DomElement("button").set_attr({ {"class", "obj-indent"}, 
                                                    {"data-type", "objective"},
                                                    {"data-parent", std::to_string(current_project->id)},
                                                    {"onclick", "addElement(this)"}})
                        .append("Add objective")
                );

            

        }

        //TODO: extract this to func/method in html_renderer
        HTML_Renderer renderer(base_body);

        size_t pos = msg->find(to_find);

        std::string rendered_str = renderer.render();

        std::cout << "rendered string: " << rendered_str << '\n';

        if(pos != std::string::npos)
        {
            std::cout << "found body mark\n";
            msg->replace(pos, to_find.size(), rendered_str);
        }

    }
    else 
    {
        std::cout << "query body is empty";
    }

    auto base_list = DomElement("");

    for(auto& proj : projects)
    {
        base_list.append(
            DomElement("p")
                .append(
                    DomElement("a").set_attr({  {"href", ("?project=" + proj.name)} })
                        .append(proj.name)
                )
            );
    }

    base_list
        .append(
            DomElement("button").set_attr({ {"class", "proj-btn"},
                                            {"data-type", "project"},
                                            {"onclick", "addElement(this)"}})
                .append("Add project")
        );

    HTML_Renderer renderer(base_list);

    std::string to_find = "{{LIST}}";
    size_t pos = msg->find(to_find);

    if(pos != std::string::npos)
    {
        msg->replace(pos, to_find.size(), renderer.render());
    }

    std::ofstream f("index_debug.html");
    f << *msg;
    f.close();
}


void update_projects(std::string *msg, std::string *query, ProjectArchive *proj_arch)
{
    JSON::JSON_Parser parser;
    auto json = parser.parse(*msg);

    long id = json["id"].get_as<double>();
    std::string value = json["value"].get_as<std::string>();


    auto it = proj_arch->elements.find(id);
    if (it == proj_arch->elements.end())
    {
        std::cerr << "update_projects: Unknown id " << std::to_string(id) << "\n";
        return;
    }

    auto& elem = it->second;

    if(auto ptr = std::get_if<Objective*>(&elem))
    {
        (*ptr)->name = value;
        (*ptr)->priority = static_cast<Priority>(json["priority"].get_as<double>());


    }
    else if(auto ptr = std::get_if<Task*>(&elem))
    {
        (*ptr)->description = value;
        (*ptr)->finished = json["finished"].get_as<bool>();
    }
    else
    {
        std::cerr << "Unknow element type\n";
        return;
    }

    save_projects_json(PROJECTS_PATH, proj_arch);
}

void add_element(std::string *msg, std::string *query, ProjectArchive *proj_arch)
{
    IdHandler id_handler(&proj_arch->projects);

    JSON::JSON_Parser parser;
    auto json = parser.parse(*msg);

    std::string& elem_type = json["type"].get_as<std::string>();
    std::string& value = json["value"].get_as<std::string>();

    if (elem_type == "project")
    {
        long id = id_handler.generate_id();

        proj_arch->projects.emplace_back(id, value);
        Project* p = &proj_arch->projects.back();

        proj_arch->elements.try_emplace(id, p);
    }
    else if (elem_type == "objective")
    {
        long id = id_handler.generate_id();
        Priority prio = static_cast<Priority>(
            json["priority"].get_as<double>()
        );

        long parent_id = static_cast<long>(
            json["parentID"].get_as<double>()
        );

        auto it = proj_arch->elements.find(parent_id);

        if (it == proj_arch->elements.end())
        {
            std::cerr << "Unknown parent ID\n";
            return;
        }

        auto parent_ptr = std::get_if<Project*>(&it->second);

        if (parent_ptr == nullptr)
        {
            std::cerr << "Expected parent PROJECT\n";
            return;
        }

        Project* parent = *parent_ptr;

        parent->objectives.emplace_back(id, value, prio);
        Objective* o = &parent->objectives.back();

        proj_arch->elements.try_emplace(id, o);
    }
    else if (elem_type == "task")
    {
        long id = id_handler.generate_id();

        long parent_id = static_cast<long>(
            json["parentID"].get_as<double>()
        );

        auto it = proj_arch->elements.find(parent_id);

        if (it == proj_arch->elements.end())
        {
            std::cerr << "Unknown parent ID\n";
            return;
        }

        auto parent_ptr = std::get_if<Objective*>(&it->second);

        if (parent_ptr == nullptr)
        {
            std::cerr << "Expected parent OBJECTIVE\n";
            return;
        }

        Objective* parent = *parent_ptr;

        parent->tasks.emplace_back(id, value);
        Task* t = &parent->tasks.back();

        proj_arch->elements.try_emplace(id, t);
    }
    else
    {
        std::cerr << "Unexpected element type " << elem_type << '\n';
        return;
    }

    save_projects_json(PROJECTS_PATH, proj_arch);
}

void delete_element(std::string *msg, std::string *query, ProjectArchive *proj_arch)
{
    JSON::JSON_Parser parser;
    auto json = parser.parse(*msg);

    long id = json["id"].get_as<double>();
    long projID = json["projID"].get_as<double>();

    auto it = proj_arch->elements.find(id);
    if (it == proj_arch->elements.end())
    {
        std::cerr << "update_projects: Unknown id " << std::to_string(id) << "\n";
        return;
    }

    auto& elem = it->second;

    //for now projects are not supported
    if(auto ptr = std::get_if<Objective*>(&elem))
    {
        std::cout << "delete_element(): Found objective even if it was not supported\n";
    }
    else if(auto ptr = std::get_if<Task*>(&elem))
    {
        long objID = json["objID"].get_as<double>();

        for(auto& p : proj_arch->projects)
        {
            if(p.id == projID)
            {
                for(auto& o : p.objectives)
                {
                    if(o.id == objID)
                    {
                        std::erase(o.tasks, (**ptr));
                    }
                }
            }
        }

        proj_arch->elements.erase(it);
    }
    else
    {
        std::cerr << "Unknow element type\n";
        return;
    }

    save_projects_json(PROJECTS_PATH, proj_arch);
}