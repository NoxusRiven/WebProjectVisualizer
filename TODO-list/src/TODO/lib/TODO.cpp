#include "TODO.hpp"
#include <string_view>
#include <iostream>

std::string to_string(Priority p)
{
    switch (p)
    {
    case Priority::LOW:
        return "LOW";

    case Priority::MEDIUM:
        return "MEDIUM";
        
    case Priority::HIGH:
        return "HIGH";

    default:
        return "NULL";
    }
}

Task::Task(long id, std::string desc) : id(id), finished(false), description(desc) {}

void Task::mark_finished()
{
    this->finished = true;
}

void Task::mark_unfinished()
{
    this->finished = false;
}



Objective::Objective(long id, std::string name, Priority priority) : id(id), name(name), priority(priority)
{
    tasks.reserve(16);
}


//TODO: fix id in this method
void Objective::add(Task task)
{
    
    this->tasks.push_back(task);
}

Project::Project(long id, std::string name) : id(id), name(name)
{
    objectives.reserve(16);
}

void Project::print()
{
    
    std::cout << "Project: " << name << "\n";

    for (const auto& obj : objectives)
    {
        std::cout << "  - Objective: " << obj.name << "\n";
        std::cout << "    * Priority: " << std::to_string(int(obj.priority)) << "\n";

        std::cout << "    * Tasks:\n";
        for (const auto& task : obj.tasks)
        {
            std::cout << "      > Description: "
                    << task.description << "\n";
            std::cout << "      > Finished: "
                    << (task.finished ? "true" : "false") << "\n\n";
        }

        std::cout << "\n";
    }

}

IdHandler::IdHandler(std::vector<Project> const *projects)
{
    max_id = 0;
    for(auto proj : *projects)
    {
        if(proj.id > max_id)
        {
            max_id = proj.id;
        }

        for(auto obj : proj.objectives)
        {
            if(obj.id > max_id) {
                max_id = obj.id; 
            }

            for(auto task : obj.tasks)
            {
                if(task.id > max_id) {
                    max_id = task.id;
                }
            }
        }
    }

    std::cout << "max id is: " << max_id << "\n";
}

long IdHandler::generate_id()
{
    return ++max_id;
}
