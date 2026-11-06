#include "DOM_Generator.hpp"
#include <unordered_set>

// ########################################################################
// ###                          DOM_ELEMENT                             ###
// ########################################################################

DomElement::DomElement(std::string value)
{
    type = value;
}

DomElement::DomElement(const DomElement& other)
{
    type = other.type;
    attrs = other.attrs;

    for (const auto& child : other.children)
    {
        if (std::holds_alternative<std::unique_ptr<DomElement>>(child))
        {
            children.emplace_back(
                std::make_unique<DomElement>(
                    *std::get<std::unique_ptr<DomElement>>(child)
                )
            );
        }
        else
        {
            children.emplace_back(
                std::get<std::string>(child)
            );
        }
    }
}

DomElement& DomElement::append(DomElement child)
{
    children.emplace_back(
        std::make_unique<DomElement>(std::move(child))
    );

    return *this;
}

DomElement& DomElement::append(std::string text)
{
    children.emplace_back(text);

    return *this;
}

DomElement& DomElement::set_attr(std::vector<Attribute> key_value)
{
    this->attrs = key_value;
    
    return *this;
}

void DomElement::clear()
{
    children.clear();
    attrs.clear();
}

// ########################################################################
// ###                          HTML_PARSER                             ###
// ########################################################################

HTML_Renderer::HTML_Renderer(DomElement &root): root(root) {}


bool HTML_Renderer::is_void(DomElement &ele)
{
    static const std::unordered_set<std::string> void_elements =
    {
        "area",
        "base",
        "br",
        "col",
        "embed",
        "hr",
        "img",
        "input",
        "link",
        "meta",
        "param",
        "source",
        "track",
        "wbr"
    };

    return void_elements.contains(ele.type);
}
    
std::string HTML_Renderer::render()
{
    return render_element(root);
}

std::string HTML_Renderer::render_element(DomElement& ele)
{
    std::string ret = {};

    bool has_parent = ele.type == "";
    if(!has_parent)
    {
        ret = "<" + ele.type;

        for (auto& a : ele.attrs)
        {
            if(std::holds_alternative<std::pair<std::string, std::string>>(a.value))
            {
                auto& pair = std::get<std::pair<std::string, std::string>>(a.value);
                ret += " " + pair.first + "=\"" + pair.second + "\"";
            }
            else
            {
                auto& single = std::get<std::string>(a.value);
                ret += " " + single;
            }
        }

        ret += ">\n";

        if (is_void(ele))
            return ret;
    }

    for (auto& child : ele.children)
    {
        if (std::holds_alternative<std::unique_ptr<DomElement>>(child))
        {
            ret += render_element(
                *std::get<std::unique_ptr<DomElement>>(child).get()
            );
        }
        else
        {
            ret += std::get<std::string>(child) + '\n';
        }
    }

    if(!has_parent)
        ret += "</" + ele.type + ">\n";

    return ret;
}