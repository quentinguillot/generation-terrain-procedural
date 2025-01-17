#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include "DataStructure/Vector3.h"
template <class T>
class GraphNode
{
public:
    GraphNode();
    GraphNode(T value);
    GraphNode(T value, const Vector3& pos, int index = 0);

    void addNeighbor(std::shared_ptr<GraphNode<T>> neighbor);
    void addNeighbor(std::shared_ptr<GraphNode<T>> neighbor, float distance);

    void removeNeighbor(std::shared_ptr<GraphNode<T>> neighbor);

    bool hasNeighbor(std::shared_ptr<GraphNode<T> > neighbor);

    float getNeighborDistanceByIndex(int index);

    Vector3 pos;
    T value;
    int index;

    Vector3 privateVector;
    int privateIndex;

    std::vector<std::pair<std::shared_ptr<GraphNode<T>>, float>> neighbors;
};

template<class T>
GraphNode<T>::GraphNode() : GraphNode(T(), Vector3(), 0)
{

}
template<class T>
GraphNode<T>::GraphNode(T value) : GraphNode(value, Vector3(), 0)
{

}
template<class T>
GraphNode<T>::GraphNode(T value, const Vector3& pos, int index)
    : value(value), pos(pos), index(index)
{

}

template<class T>
void GraphNode<T>::addNeighbor(std::shared_ptr<GraphNode<T>> neighbor)
{
    this->neighbors.push_back(std::make_pair(neighbor, (this->pos - neighbor->pos).norm()));
}

template<class T>
void GraphNode<T>::addNeighbor(std::shared_ptr<GraphNode<T> > neighbor, float distance)
{
    this->neighbors.push_back(std::make_pair(neighbor, distance));
}

template<class T>
void GraphNode<T>::removeNeighbor(std::shared_ptr<GraphNode<T>> neighbor)
{
    for (int i = this->neighbors.size() - 1; i >= 0; i--)
        if (this->neighbors[i].first == neighbor)
            this->neighbors.erase(this->neighbors.begin() + i);
//    this->neighbors.erase(std::find(this->neighbors.begin(), this->neighbors.end(), neighbor));
}

template<class T>
bool GraphNode<T>::hasNeighbor(std::shared_ptr<GraphNode<T>> neighbor)
{
    for (auto& [n, w] : this->neighbors)
        if (n == neighbor)
            return true;
    return false;
}

template<class T>
float GraphNode<T>::getNeighborDistanceByIndex(int index)
{
    float distance = 0.f;
    bool neighborFound = false;
    for (auto& neighbor : this->neighbors) {
        if (std::get<0>(neighbor)->index == index) {
            neighborFound = true;
            distance += std::get<1>(neighbor);
        }
    }
    if (neighborFound)
        return distance;
    return std::numeric_limits<float>::max();
}

#endif // GRAPHNODE_H
