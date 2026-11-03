#include <iostream>

#include "TODO.hpp"

int main()
{
    std::string task = "Simple task to add";
    std::string comment = "//comment to task";
    std::string comment_space = "// comment with space";
    //Priority priority = LOW
    Objective obj{"New Objective", Priority::LOW};


    obj.add(task);
    obj.add(comment);
    obj.add(comment_space);

    std::cout << "Tasks:\n";
    for(auto task : obj.tasks)
    {
        std::cout << task.description << "\n";
    }

    std::cout << "Comments\n";
    for(auto comm : obj.comments)
    {
        std:: cout << comm << '\n';
    }

    return 0;
}