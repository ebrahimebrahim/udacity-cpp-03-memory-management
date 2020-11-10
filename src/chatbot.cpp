#include <iostream>
#include <random>
#include <algorithm>
#include <ctime>

#include "chatlogic.h"
#include "graphnode.h"
#include "graphedge.h"
#include "chatbot.h"

// constructor WITHOUT memory allocation
ChatBot::ChatBot()
{
    // invalidate data handles
    _chatLogic = nullptr;
    _rootNode = nullptr;
}

// constructor WITH memory allocation
ChatBot::ChatBot(std::string filename)
{
    std::cout << "ChatBot Constructor" << std::endl;
    
    // invalidate data handles
    _chatLogic = nullptr;
    _rootNode = nullptr;

    // load image into heap memory
    _image = std::make_unique<wxBitmap>(filename, wxBITMAP_TYPE_PNG);
}

ChatBot::~ChatBot()
{
    std::cout << "ChatBot Destructor" << std::endl;
}

//// STUDENT CODE
////
// (Task 2)

ChatBot::ChatBot(const ChatBot & src){
    std::cout << "ChatBot Copy Constructor" << std::endl;
    
    // copy data handles
    _chatLogic = src._chatLogic;
    _rootNode = src._rootNode;

    // use copy ctor of wxBitmap to make a new _image in the heap
    _image = std::make_unique<wxBitmap>(*(src._image));
    /* Note that this does not make a true copy of the underlying image, it is only copying
    the wxBitmap object. This is because of the way wxWidgets treats its copy ctor; see here:
    https://docs.wxwidgets.org/3.0/classwx_bitmap.html#abfaa21ec563a64ea913af918150db900
    We *could* make a real copy using wxBitmap::GetSubBitmap, but I don't wnat the chatbot
    memory management philosophy to be fighting against that of a library that it is using.
    I'm going to accept this situation as being a quirk that is internal to wxWidgets.
    Someone using this chatbot would have to know that this is just how wxWidgets images get "copied."
    */
}

ChatBot & ChatBot::operator=(const ChatBot & src){
    std::cout << "ChatBot Copy Assignment" << std::endl;
    
    if (&src == this) return *this;

    // copy data handles
    _chatLogic = src._chatLogic;
    _rootNode = src._rootNode;

    // use copy ctor of wxBitmap to make a new _image in the heap
    _image.reset(new wxBitmap(*(src._image)));
    // Again, not a real copy of the underlying image. See above.

    return *this;
}

ChatBot::ChatBot(ChatBot && src){
    std::cout << "ChatBot Move Constructor" << std::endl;
    
    // copy data handles
    _chatLogic = src._chatLogic;
    _rootNode = src._rootNode;

    // use move assignment of unqiue_ptr to steal the _image resouce, leaving src with an invalid _image
    _image = std::move(src._image);
}

ChatBot & ChatBot::operator=(ChatBot && src){
    std::cout << "ChatBot Move Assignment" << std::endl;
    
    if (&src == this) return *this;
    // Should we protect against self move-assignment?? Really weird situation.

    // copy data handles
    _chatLogic = src._chatLogic;
    _rootNode = src._rootNode;

    // use move assignment of unqiue_ptr to steal the _image resouce, leaving src with an invalid _image
    _image = std::move(src._image);

    return *this;
}

void ChatBot::updateChatLogicWithSelf() {
    _chatLogic->SetChatbotHandle(this);
}

////
//// EOF STUDENT CODE

void ChatBot::ReceiveMessageFromUser(std::string message)
{
    // loop over all edges and keywords and compute Levenshtein distance to query
    typedef std::pair<GraphEdge *, int> EdgeDist;
    std::vector<EdgeDist> levDists; // format is <ptr,levDist>

    for (size_t i = 0; i < _currentNode->GetNumberOfChildEdges(); ++i)
    {
        GraphEdge *edge = _currentNode->GetChildEdgeAtIndex(i);
        for (auto keyword : edge->GetKeywords())
        {
            EdgeDist ed{edge, ComputeLevenshteinDistance(keyword, message)};
            levDists.push_back(ed);
        }
    }

    // select best fitting edge to proceed along
    GraphNode *newNode;
    if (levDists.size() > 0)
    {
        // sort in ascending order of Levenshtein distance (best fit is at the top)
        std::sort(levDists.begin(), levDists.end(), [](const EdgeDist &a, const EdgeDist &b) { return a.second < b.second; });
        newNode = levDists.at(0).first->GetChildNode(); // after sorting the best edge is at first position
    }
    else
    {
        // go back to root node
        newNode = _rootNode;
    }

    // tell current node to move chatbot to new node
    _currentNode->MoveChatbotToNewNode(newNode);
}

void ChatBot::SetCurrentNode(GraphNode *node)
{
    // update pointer to current node
    _currentNode = node;

    // select a random node answer (if several answers should exist)
    std::vector<std::string> answers = _currentNode->GetAnswers();
    std::mt19937 generator(int(std::time(0)));
    std::uniform_int_distribution<int> dis(0, answers.size() - 1);
    std::string answer = answers.at(dis(generator));

    // send selected node answer to user
    _chatLogic->SendMessageToUser(answer);
}

int ChatBot::ComputeLevenshteinDistance(std::string s1, std::string s2)
{
    // convert both strings to upper-case before comparing
    std::transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
    std::transform(s2.begin(), s2.end(), s2.begin(), ::toupper);

    // compute Levenshtein distance measure between both strings
    const size_t m(s1.size());
    const size_t n(s2.size());

    if (m == 0)
        return n;
    if (n == 0)
        return m;

    size_t *costs = new size_t[n + 1];

    for (size_t k = 0; k <= n; k++)
        costs[k] = k;

    size_t i = 0;
    for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
    {
        costs[0] = i + 1;
        size_t corner = i;

        size_t j = 0;
        for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
        {
            size_t upper = costs[j + 1];
            if (*it1 == *it2)
            {
                costs[j + 1] = corner;
            }
            else
            {
                size_t t(upper < corner ? upper : corner);
                costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
            }

            corner = upper;
        }
    }

    int result = costs[n];
    delete[] costs;

    return result;
}