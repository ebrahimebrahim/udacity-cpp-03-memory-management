#include "graphedge.h"
#include "graphnode.h"

GraphNode::GraphNode(int id)
{
    _id = id;
}

GraphNode::~GraphNode()
{
    //// STUDENT CODE
    ////

    // Task 0: The commented out line below was the memory bug.
    // The ChatLogic is the owner of _chatBot, so GraphNode has no business deleting _chatBot.
    // (Note: the above statement is no longer true after Task 5 is completed)
    // delete _chatBot;

    ////
    //// EOF STUDENT CODE
}

void GraphNode::AddToken(std::string token)
{
    _answers.push_back(token);
}

void GraphNode::AddEdgeToParentNode(GraphEdge *edge)
{
    _parentEdges.push_back(edge);
}

void GraphNode::AddEdgeToChildNode(std::unique_ptr<GraphEdge> && edge) // Task 4
{
    _childEdges.emplace_back(std::move(edge));
}

//// STUDENT CODE
//// Task 5
void GraphNode::MoveChatbotHere(ChatBot chatBot)
{
    _chatBot = std::move(chatBot);
    _chatBot.updateChatLogicWithSelf();
    _chatBot.SetCurrentNode(this);
}

void GraphNode::MoveChatbotToNewNode(GraphNode *newNode)
{
    newNode->MoveChatbotHere(std::move(_chatBot));
}
////
//// EOF STUDENT CODE

GraphEdge *GraphNode::GetChildEdgeAtIndex(int index)
{
    //// STUDENT CODE
    //// Task 4

    return _childEdges[index].get();

    ////
    //// EOF STUDENT CODE
}