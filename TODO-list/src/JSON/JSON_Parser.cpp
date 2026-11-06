#include "JSON_Parser.hpp"
#include <string>
#include <sstream>


namespace JSON
{
    // ------------------------------------------ JSON VALUE ------------------------------------------
    std::string JsonValue::to_string()
    {
        if (std::holds_alternative<std::string>(_data))
        {
            return std::get<std::string>(_data);
        }
        return "not string";

    }


    JsonValue& JsonValue::operator[](const std::string& key)
    {
        return std::get<j_object>(_data)[key];
    }

    JsonValue& JsonValue::operator[](size_t idx)
    {
        return std::get<j_array>(_data)[idx];
    }

    // ------------------------------------------ JSON PARSER ------------------------------------------
    JsonValue JSON_Parser::parse_value(Token t, std::istream& stream)
    {
        switch (t.type) 
        {
            case Token::LeftBrace:
                return parse_object(stream);
            case Token::LeftBracket:
                return parse_array(stream);
            case Token::String:
                return JsonValue(t.value);
            case Token::Number:
                return JsonValue(std::stod(t.value));
            case Token::True:
                return JsonValue(true);
            case Token::False:
                return JsonValue(false);
            case Token::Null:
                return JsonValue();
            default:
                std::cerr << "Wrong token in json file: " << t.type;
                exit(-1);
        }
    }

    JsonValue JSON_Parser::parse(std::string_view data)
    {
        if (data.ends_with(".json"))
        {
            std::ifstream file(data.data());
            return parse_stream(file);
        }
        else
        {
            std::string buffer(data);
            std::stringstream ss(buffer);
            return parse_stream(ss);
        }
    }

    JsonValue JSON_Parser::parse_stream(std::istream& stream)
    {
        Token first = next_token(stream);

        if (first.type != Token::LeftBrace)
        {
            std::cerr << "JSON must start with '{'\n";
            return JsonValue();
        }

        _root = parse_object(stream);

        return _root;
    }

    Token JSON_Parser::next_token(std::istream& stream)
    {
        _token_count++;
        char c;
        // std::cout << "========================\n";  
        // std::cout << "c: " << c << "\n";
        // std::cout << "========================\n";  
        while (stream.get(c))
        {
            switch (c)
            {   
            case '{': return Token{ Token::LeftBrace };
            case '}': return Token{ Token::RightBrace };
            case '[': return Token{ Token::LeftBracket };
            case ']': return Token{ Token::RightBracket };
            case ':': return Token{ Token::Colon };
            case ',': return Token{ Token::Comma };
            }

            if (c == '"') 
            {
                std::string result;
                while (stream.get(c) && c != '"')
                {
                    result += c;
                }
                return {Token::String, result};
            }

            if (isdigit(c) || c == '-')
            {
                std::string result(1, c);

                bool dot_encountered = false;
                while (isdigit(stream.peek()) ||
                       (stream.peek() == '.' && !dot_encountered))
                {
                    stream.get(c);
                    if (c == '.')
                        dot_encountered = true;
                    result += c;
                }
                return {Token::Number, result};
            }

            if (c == 't')
            {
                if (stream.get() == 'r' && stream.get() == 'u' && stream.get() == 'e')
                    return {Token::True};
                else
                    throw std::runtime_error("Invalid token: expected 'true'");
                
            }

            if (c == 'f')
            {
                if (stream.get() == 'a' && stream.get() == 'l' && stream.get() == 's' && stream.get() == 'e')
                    return {Token::False};
                else
                    throw std::runtime_error("Invalid token: expected 'false'");
                
            }

            if (c == 'n')
            {
                if (stream.get() == 'u' && stream.get() == 'l' && stream.get() == 'l')
                    return {Token::Null};
                else
                    throw std::runtime_error("Invalid token: expected 'null'");
            }

        }
        return Token{ Token::End };
    }

    JsonValue JSON_Parser::parse_object(std::istream& stream)
    {
        JsonValue newJsonObject(j_object{});
        
        //making JsonValue an object and storing it in temp var
        j_object& obj = newJsonObject.get_as<j_object>();
        Token key, colon, t_value;
        JsonValue j_value;
        Token checkIfEnd = next_token(stream);

        if(checkIfEnd.type == Token::RightBrace)
        {
            return newJsonObject;
        }

        //if not empty object, it should contain key so copy it to key token
        key = checkIfEnd;
        if (key.type != Token::String)
        {
            std::cerr << "Key in json object should be a string\n";
            std::cerr << "Found token type: " << key.type << ", in line: "<< _token_count << "\n";
            exit(-1);
        }

        colon = next_token(stream);
        if (colon.type != Token::Colon)
        {
            std::cerr << "Key and value should be separated by ':'\n";
            std::cerr << "Found token type: " << colon.type << ", in line: "<< _token_count << "\n";
            exit(-1);

        }

        t_value = next_token(stream);
        j_value = parse_value(t_value, stream);
        obj.insert({key.value, j_value});

        
        checkIfEnd = next_token(stream);
        while (checkIfEnd.type != Token::RightBrace)
        {
            if (checkIfEnd.type != Token::Comma)
            {
                std::cerr << "At the end of the key value pair expected ',' or '}'\n";
                std::cerr << "Found token type: " << checkIfEnd.type << ", in line: "<< _token_count << "\n";
                std::cerr << "key value: " << key.value << "\n";
                std::cerr << "json value: " << j_value.to_string() << "\n";
                exit(-1);
            }

            key = next_token(stream);
            if (key.type != Token::String)
            {
                std::cerr << "Key in json object should be a string\n";
                std::cerr << "Found token type: " << key.type << ", in line: "<< _token_count << "\n";
                exit(-1);
            }

            colon = next_token(stream);
            if (colon.type != Token::Colon)
            {
                std::cerr << "Key and value should be separated by ':'\n";
                std::cerr << "Found token type: " << colon.type << ", in line: "<< _token_count << "\n";
                exit(-1);

            }

            t_value = next_token(stream);
            j_value = parse_value(t_value, stream);
            obj.insert({key.value, j_value});

            checkIfEnd = next_token(stream);
        }
        
        return newJsonObject;
    }

    JsonValue JSON_Parser::parse_array(std::istream& stream)
    {
        JsonValue newJsonArray(j_array{});
        
        //making JsonValue an object and storing it in temp var
        j_array& arr = newJsonArray.get_as<j_array>();

        JsonValue value; 
        Token checkIfEnd = next_token(stream);

        if(checkIfEnd.type == Token::RightBracket)
        {
            return newJsonArray;
        }

        //if code reaches here, checkIfEnd is first value
        //this will handle errors if next token is not a value
        value = parse_value(checkIfEnd, stream);
        
        arr.push_back(value);

        checkIfEnd = next_token(stream);
        while (checkIfEnd.type != Token::RightBracket)
        {
            if (checkIfEnd.type != Token::Comma)
            {
                std::cerr << "At the end of the element ',' or ']'\n";
                std::cerr << "Found token type: " << checkIfEnd.type << ", in line: "<< _token_count << "\n";
                exit(-1);
            }

            //adding next key value pair if ',' is present
            Token t = next_token(stream);
            JsonValue nextValue = parse_value(t, stream);

            arr.push_back(nextValue);

            checkIfEnd = next_token(stream);
        }

        return newJsonArray;
    }
}
