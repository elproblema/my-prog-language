#include <ast.h>
#include <SC.h>

#define SC SuperCombinator
// Only lambda overloads
std::vector<std::shared_ptr<Node>> LambdaNode::GetChildren() {
    if (ind_tag) return body->GetChildren();
    return {body};
}

const std::vector<std::shared_ptr<VarNode>>& LambdaNode::GetBonded() const { 
    if (ind_tag) return body->GetBonded();
    return bonded;
}

std::shared_ptr<const Node> LambdaNode::GetFunc() const { 
    if (ind_tag) return body->GetFunc();
    return nullptr; 
}

std::shared_ptr<Node> LambdaNode::GetFunc() { 
    if (ind_tag) return body->GetFunc();
    return nullptr; 
}

std::shared_ptr<const Node> LambdaNode::GetArg() const { 
    if (ind_tag) return body->GetArg();
    return nullptr; 
}

std::shared_ptr<Node> LambdaNode::GetArg() { 
    if (ind_tag) return body->GetArg();
    return nullptr; 
}

std::shared_ptr<const Node> LambdaNode::GetBody() const { 
    if (ind_tag) return body->GetBody();
    return body; 
}

std::shared_ptr<Node> LambdaNode::GetBody() { 
    if (ind_tag) return body->GetBody();
    return body; 
}

const std::vector<std::weak_ptr<VarNode>>& 
LambdaNode::GetSomeVars() const { 
    if (ind_tag) return body->GetSomeVars();
    throw WAE("GetSomeVars"); 
}

const std::string& LambdaNode::GetName() const { 
    if (ind_tag) return body->GetName();
    throw WAE("GetName"); 
}

const std::string& LambdaNode::GetFuncName() const { 
    if (ind_tag) return body->GetFuncName();
    throw WAE("GetFuncName"); 
}


void LambdaNode::SetFreeVar() {
    if (ind_tag) body->SetFreeVar();
}

void LambdaNode::SetBounded() {
    if (ind_tag) body->SetBounded();
    for (auto var : bonded) {
        var->head = weak_from_this();
    }
}

void LambdaNode::Substitute() {
    if (ind_tag) body->Substitute();
    body = SC::Substitution(shared_from_this());
    SetIndirected();
}

bool LambdaNode::IsFunc() const {
    if (ind_tag) return body->IsFunc();
    throw "must be indirected";
}

std::shared_ptr<Node> LambdaNode::EtaConversion() { 
    if (ind_tag) return body->EtaConversion();
    throw WAE("EtaConversion"); 
}

std::weak_ptr<LambdaNode> LambdaNode::GetHead() { 
    if (ind_tag) return body->GetHead();
    throw WAE("GetHead"); 
}
