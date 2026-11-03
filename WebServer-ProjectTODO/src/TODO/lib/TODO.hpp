#include <string>
#include <vector>

enum class Priority
{
    LOW = 0,
    MEDIUM,
    HIGH,
};

std::string to_string(Priority p);

class Task
{
public:
    long id;

    bool finished;
    std::string description; 

    Task(long id, std::string desc);

    void mark_finished();
    void mark_unfinished();
};

struct Comment
{
    long id;

    std::string text;

    Comment(long id, std::string content);
};

class Objective
{
public:
    long id;

    std::string name;
    Priority priority;
    std::vector<Comment> comments;
    std::vector<Task> tasks;

    Objective(long id, std::string name, Priority priority);

    // check if types are either string (comment) or Task commes shoud be prefixed with '//'
    void add(std::string data);

};

class Project
{
public:
    long id;

    std::string name;
    std::vector<Objective> objectives;

    Project(long id, std::string name);
    Project(){}
    
    void print();
};

class IdHandler
{
public:
    long max_id;

    IdHandler(std::vector<Project> const * projects);

    long generate_id();
};