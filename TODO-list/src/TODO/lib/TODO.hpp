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

    //needed for erase
    bool operator==(const Task& other) const {
        return this->id == other.id;
    }
};

class Objective
{
public:
    long id;

    std::string name;
    Priority priority;
    std::vector<Task> tasks;

    Objective(long id, std::string name, Priority priority);

    void add(Task task);
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