#include "expand_repeats.h"
#include <unordered_map>

using std::string;
using std::to_string;
using std::unordered_map;
using namespace tree_sitter::rules;

namespace tree_sitter {
    namespace prepare_grammar {
        class RepeatExpander : rules::Visitor {
        public:
            rule_ptr value;
            unordered_map<string, const rule_ptr> aux_rules;
            
            rule_ptr apply(const rule_ptr rule) {
                rule->accept(*this);
                return value;
            }
            
            rule_ptr make_repeat_helper(string name, const rule_ptr &rule) {
                return seq({
                    rule,
                    choice({ sym(name), blank() })
                });
            }
            
            void visit(const Repeat *rule) {
                rule_ptr inner_rule = apply(rule->content);
                string helper_rule_name = string("repeat_helper") + to_string(aux_rules.size() + 1);
                aux_rules.insert({ helper_rule_name, make_repeat_helper(helper_rule_name, inner_rule) });
                value = sym(helper_rule_name);
            }
            
            void visit(const Seq *rule) {
                value = seq({ apply(rule->left), apply(rule->right) });
            }

            void visit(const Choice *rule) {
                value = choice({ apply(rule->left), apply(rule->right) });
            }

            void default_visit(const Rule *rule) {
                value = rule->copy();
            }
        };
        
        Grammar expand_repeats(const Grammar &grammar) {
            unordered_map<string, const rule_ptr> result;
            RepeatExpander visitor;
            for (auto pair : grammar.rules)
                result.insert({ pair.first, visitor.apply(pair.second) });
            for (auto pair : visitor.aux_rules)
                result.insert(pair);
            return Grammar(grammar.start_rule_name, result);
        }
    }
}