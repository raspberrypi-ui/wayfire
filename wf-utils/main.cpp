#include "wayfire/lexer/lexer.hpp"
#include "wayfire/parser/condition_parser.hpp"
#include "wayfire/parser/rule_parser.hpp"
#include "wayfire/rule/rule.hpp"
#include "wayfire/action/action_interface.hpp"
#include "wayfire/condition/access_interface.hpp"
#include "wayfire/variant.hpp"
#include "wayfire/parser/lambda_rule_parser.hpp"
#include "wayfire/rule/lambda_rule.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class test_t
{
public:
    void maximize()
    {
        std::cout << "void maximize()" << std::endl;
    }

    void minimize()
    {
        std::cout << "void minimize()" << std::endl;
    }

    int property_a;

    double get_property_b() const
    {
        std::cout << "double get_property_b()" << std::endl;
        return _property_b;
    }

    void set_property_b(double property_b)
    {
        std::cout << "void set_property_b(" << property_b << ")" << std::endl;
        _property_b = property_b;
    }

    std::string title;
private:
    double _property_b;
};

class test_access_interface_t : public wf::access_interface_t
{
public:
    test_access_interface_t(std::shared_ptr<test_t> test) : _test(test)
    {
    }

    virtual ~test_access_interface_t() override;

    virtual wf::variant_t get(const std::string &identifier, bool &error) override
    {
        error = false;
        if (_test != nullptr)
        {
            if (identifier == "property_a")
            {
                return _test->property_a;
            }
            else if (identifier == "property_b")
            {
                return _test->get_property_b();
            }
            else if (identifier == "title")
            {
                return _test->title;
            }
        }

        error = true;
        return wf::variant_t("");
    }
private:
    std::shared_ptr<test_t> _test;
};

class test_action_interface_t : public wf::action_interface_t
{
public:
    test_action_interface_t(std::shared_ptr<test_t> test) : _test(test)
    {
    }

    virtual ~test_action_interface_t() override;

    virtual bool execute(const std::string &name, const std::vector<wf::variant_t> &args) override
    {
        auto error = _test == nullptr;
        if (!error)
        {
            if ((name == "set") && (args.size() == 2))
            {
                auto property = wf::get_string(args.at(0));
                if (property == "property_a")
                {
                    _test->property_a = wf::get_int(args.at(1));
                }
                else if (property == "property_b")
                {
                    _test->set_property_b(wf::get_double(args.at(1)));
                }
                else
                {
                    error = true;
                }
            }
            else if (name == "maximize")
            {
                _test->maximize();
            }
            else if (name == "minimize")
            {
                _test->minimize();
            }
            else
            {
                error = true;
            }
        }
        return error;
    }
private:
    std::shared_ptr<test_t> _test;
};

test_access_interface_t::~test_access_interface_t()
{
}

test_action_interface_t::~test_action_interface_t()
{
}

bool if_function()
{
    std::cout << "IF_FUNCTION!" << std::endl;
    return false;
}

bool else_function()
{
    std::cout << "ELSE_FUNCTION!" << std::endl;
    return false;
}

int main()
{
    /*
    lexer_t lexer("(a equals 3 | a equals 2) & (! x equals 4)");
    condition_parser_t parser;

    try
    {
        auto condition = parser.parse(lexer);
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Malformed input:" << std::endl;
        std::cout << lexer.text() << std::endl;
        std::string pad;
        auto pos = lexer.current_symbol_position();
        for (std::size_t i = 0; i < pos; ++i)
        {
            pad.append(" ");
        }
        pad.append("^ ");
        std::cout << pad << e.what() << std::endl;

    }
    */

    /*
    std::string text_reverse = "one two three four five";

    auto data = std::make_shared<test_t>();
    data->property_a = 4;
    data->title = "Alacritty";

    test_access_interface_t access_interface(data);
    test_action_interface_t action_interface(data);

    std::vector<std::string> text = {
        "on created if property_a equals 4 then set property_b 0.4",
        "on created if (property_a equals 4) | (property_a equals 8) then maximize else minimize",
        "on created then minimize",
        "on created then maximize else minimize",
        "on created if title contains \"   Alacritty   \" then maximize"
    };

    wf::lexer_t lexer;
    wf::rule_parser_t rule_parser;

    std::vector<std::shared_ptr<wf::rule_t>> rules;

    for (const auto &t : text)
    {
        lexer.reset(t);
        rules.push_back(rule_parser.parse(lexer));
    }

    for (auto rule : rules)
    {
        rule->apply("created", access_interface, action_interface);
    }
    */

    auto data = std::make_shared<test_t>();
    data->property_a = 4;
    data->title = "Alacritty";

    test_access_interface_t access_interface(data);
//    test_action_interface_t action_interface(data);

//    std::vector<std::string> text = {
//        "property_a equals 4",
//        "all",
//        "none",
//        "then",
//        "title contains \"   Alacritty   \""
//    };

//    wf::lexer_t lexer;
//    wf::condition_parser_t parser;

//    std::vector<std::shared_ptr<wf::condition_t>> conditions;

//    for (const auto &t : text)
//    {
//        lexer.reset(t);
//        conditions.push_back(parser.parse(lexer));
//    }

    std::string text = "on fullscreened if title contains \"Alacritty\" & property_a equals 4";
    wf::lambda_rule_parser_t parser;

    auto rule = parser.parse(text, &if_function, &else_function);

    std::cout << "rule: " << rule->to_string() << std::endl;

    auto error = rule->apply("fullscreened", access_interface);
    if (error)
    {
        std::cout << "Error!" << std::endl;
    }


    /*

    lexer.reset(text_reverse);

    auto s = lexer.parse_symbol();
    auto p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    lexer.reverse();
    p = lexer.current_symbol_position();
    std::cout << "reversed to p: " << p << std::endl;

    s = lexer.parse_symbol();
    p = lexer.current_symbol_position();
    std::cout << "p: " << p << " s: " << s.to_string() << std::endl;
*/
    return 0;
}
