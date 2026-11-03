#include <variant>
#include <string>
#include <vector>
#include <memory>

class DomElement;

using Child = std::variant<
    std::unique_ptr<DomElement>,
    std::string
>;

struct Attribute
{
    std::variant<
        std::string, 
        std::pair<std::string, std::string>
    > value;

    Attribute() {}
    
    Attribute(std::string key) 
        : value(std::move(key)) {}

    Attribute(std::string key, std::string val) 
        : value(std::pair{std::move(key), std::move(val)}) {}
};

class DomElement
{
public:
    std::vector<Child> children;
    std::vector<Attribute> attrs;
    std::string type;
    
    DomElement(std::string value);
    DomElement(const DomElement &other);

    // adds child to children list and returns self
    DomElement& append(DomElement child);
    DomElement& append(std::string text);
    
    // every time when called overrides attribute attrs, should be called once, returns itself
    DomElement& set_attr(std::vector<Attribute> attrs);

    void clear();
};

/*
void elem list:

area
base
br
col
embed
hr
img
input
link
meta
param
source
track
wbr
*/

class HTML_Renderer
{
private:
    DomElement& root;

    bool is_void(DomElement &ele);
    
    std::string render_element(DomElement &ele);
public:
    HTML_Renderer(DomElement &root);
    std::string render();
};
