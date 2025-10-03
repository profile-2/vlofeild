#define OLC_PGE_APPLICATION
#include <random>
#include "OneLoneCoder/olcPixelGameEngine.h"
#include "profile_2/p2util.h"

#define SCREEN_WIDTH    300
#define SCREEN_HEIGHT   210
#define PIXEL_SIZE      4
#define DEBUG           0

enum DIRECTIONS{
    DIR_UNDEFINED,
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
    DIR_HORIZONTAL,
    DIR_VERTICAL,
    DIR_START,
    DIR_END
};

/*
TODO:
Make target interact with trail
Win and lose conditions

bugs:
Once the program went into an infinite loop with runaway ram consuption. Couldn't replicate it yet, is most likely related with very short lines produced by backtracking while the ship is in the trail.
*/

std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<int> distColor(0, 128);
std::uniform_real_distribution<float> distSpeed(0.5,1.5);

#pragma region sPath
struct sPath{
    std::vector<olc::vf2d> nodes;
    int currentNode;
    std::vector<std::pair<olc::vi2d, olc::vi2d>> rectangles;
    float fAreaPercent;

    sPath(){
        currentNode = 0;
        fAreaPercent = 1;
    }

    bool AreColinear(const olc::vf2d& pointA, const olc::vf2d& pointB, const olc::vf2d& pointC){
        return (pointA.x == pointB.x && pointB.x == pointC.x) || (pointA.y == pointB.y && pointB.y == pointC.y); 
    }

    float CalcDistance(int nNode, const olc::vf2d& vfPoint){
        if(nodes[nNode].x == vfPoint.x){ // vertical distance
            return vfPoint.y - nodes[nNode].y;
        }
        if(nodes[nNode].y == vfPoint.y){
            return vfPoint.x - nodes[nNode].x;
        }
        return 0; 
    }

    float CalcDistance2(int nNode, const olc::vf2d& vfPoint, const std::vector<olc::vf2d>& vNodes){
        if(vNodes[nNode].y == vNodes[Next(nNode, vNodes)].y){ // vertical distance
            return std::abs(vfPoint.y - vNodes[nNode].y);
        }
        if(vNodes[nNode].x == vNodes[Next(nNode, vNodes)].x){
            return std::abs(vfPoint.x - vNodes[nNode].x);
        }
        return 0; 
    }

    bool DoesIntersect(olc::vf2d point1A, olc::vf2d point1B, olc::vf2d point2A, olc::vf2d point2B) const {
        
        float point2AX = point2A.x < point2B.x ? point2A.x : point2B.x;
        float point2BX = point2A.x > point2B.x ? point2A.x : point2B.x;
        float point2AY = point2A.y < point2B.y ? point2A.y : point2B.y;
        float point2BY = point2A.y > point2B.y ? point2A.y : point2B.y;

        float point1AX = point1A.x < point1B.x ? point1A.x : point1B.x;
        float point1BX = point1A.x > point1B.x ? point1A.x : point1B.x;
        float point1AY = point1A.y < point1B.y ? point1A.y : point1B.y;
        float point1BY = point1A.y > point1B.y ? point1A.y : point1B.y;

        if(point2AX == point2BX){
            if(point1AX <= point2AX && point2AX <= point1BX && point2AY <= point1AY && point1AY <= point2BY)
                return true;
        }
        else{
            if(point1AY <= point2AY && point2AY <= point1BY && point2AX <= point1AX && point1AX <= point2BX)
                return true;
        }
        return false;
    }

    void Draw(olc::PixelGameEngine& pge, int node, olc::Pixel color = olc::WHITE){
        pge.DrawLine(nodes[node], nodes[Next(node)], color);
    }

    void DrawAll(olc::PixelGameEngine& pge, olc::Pixel color = olc::WHITE){
        for(int i = 0; i < nodes.size(); i++){
            Draw(pge, i, color);
        }
    }

    int GetInDirection(int nNode, const std::vector<olc::vf2d>& vNodes){
        int nextNode = Next(nNode, vNodes);
        if(IsVertical(nNode, vNodes)){
            olc::vf2d testPoint = olc::vf2d(vNodes[nNode].x,(vNodes[nNode].y+vNodes[Next(nNode, vNodes)].y)/2);
            if(InDirection(DIR_LEFT, testPoint, nNode, vNodes)) return DIR_LEFT;
            if(InDirection(DIR_RIGHT, testPoint, nNode, vNodes)) return DIR_RIGHT;
        }
        else{
            olc::vf2d testPoint = olc::vf2d((vNodes[nNode].x+vNodes[Next(nNode, vNodes)].x)/2,vNodes[nNode].y);
            if(InDirection(DIR_UP, testPoint, nNode, vNodes)) return DIR_UP;
            if(InDirection(DIR_DOWN, testPoint, nNode, vNodes)) return DIR_DOWN;
        }
        return DIR_UNDEFINED;
    }
    int GetInDirection(int nNode) { return GetInDirection(nNode, nodes); }

    int GetEndExit(int node){
        if(IsVertical(node)){
            if (nodes[Next(node)].x >= nodes[Next(Next(node))].x)
                return DIR_LEFT;
            else
                return DIR_RIGHT;
        }
        else{
            if (nodes[Next(node)].y >= nodes[Next(Next(node))].y)
                return DIR_UP;
            else
                return DIR_DOWN;
        }
        return DIR_UNDEFINED;
    }

    int GetStartExit(int node){
        if(IsVertical(node)){
            if (nodes[Prev(node)].x >= nodes[node].x)
                return DIR_RIGHT;
            else
                return DIR_LEFT;
        }
        else{
            if (nodes[Prev(node)].y >= nodes[node].y)
                return DIR_DOWN;
            else
                return DIR_UP;
        }
        return DIR_UNDEFINED;
    }

    void GetRectangles(std::vector<olc::vf2d>& vNodes){
        int currentNode = 0;
        
        while(IsReflex(currentNode, vNodes)) currentNode = Prev(currentNode, vNodes);
        
        int firstNode = currentNode;
        currentNode = Next(currentNode, vNodes); 
        int nDeparture = -1;
        int nIntersect = -1;
        olc::vf2d vfIntersect;
        float fIntersectDistance = INFINITY;

        while(currentNode != firstNode && nDeparture == -1){
            if(IsReflex(currentNode, vNodes)) {
                nDeparture = currentNode;
            }
            else {
                currentNode = Next(currentNode, vNodes);
            }
        }
        
        if(nDeparture == -1 || vNodes.size() < 4){ // all non reflex, valid rectangle // escape condition in case something goes wrong -> "|| vNodes.size() < 4"
            int top = 1000;
            int left = 1000;
            int bottom = 0;
            int right = 0;
            for(auto r : vNodes){
                if(r.x < left) left = r.x;
                if(r.x > right) right = r.x;
                if(r.y < top) top = r.y;
                if(r.y > bottom) bottom = r.y;
            }
            rectangles.emplace_back(olc::vi2d(left,top),olc::vi2d(right,bottom));
            return;
        }

        int prevNode = Prev(currentNode, vNodes);

        switch(GetInDirection(prevNode, vNodes)){
            case DIR_LEFT:{ vfIntersect = olc::vf2d(vNodes[prevNode]+olc::vf2d(0,INFINITY)); break; }
            case DIR_UP:{ vfIntersect = olc::vf2d(vNodes[prevNode]+olc::vf2d(-INFINITY,0)); break; }
            case DIR_RIGHT:{ vfIntersect = olc::vf2d(vNodes[prevNode]+olc::vf2d(0,-INFINITY)); break; }
            case DIR_DOWN:{ vfIntersect = olc::vf2d(vNodes[prevNode]+olc::vf2d(INFINITY,0)); break; }
        }
        for(int i = 0; i < vNodes.size(); i++){
            if(i != Prev(prevNode, vNodes) && i != prevNode && i != currentNode){
                if(DoesIntersect(vNodes[prevNode],vfIntersect,vNodes[i],vNodes[Next(i, vNodes)])){
                    float distance = CalcDistance2(i, vNodes[prevNode], vNodes);
                    if(distance < fIntersectDistance){
                        fIntersectDistance = distance;
                        nIntersect = i;
                        if(IsVertical(prevNode, vNodes)) vfIntersect = olc::vf2d(vNodes[prevNode].x,vNodes[nIntersect].y);
                        else vfIntersect = olc::vf2d(vNodes[nIntersect].x,vNodes[prevNode].y);
                    }
                }
            }
        }
        
        std::vector<olc::vf2d> polygonA;
        std::vector<olc::vf2d> polygonB;

        polygonA.emplace_back(vNodes[prevNode]);
        polygonA.emplace_back(vfIntersect);
        currentNode = Next(nIntersect, vNodes);
        while(currentNode != prevNode){
            polygonA.emplace_back(vNodes[currentNode]);
            currentNode = Next(currentNode, vNodes);
        }

        polygonB.emplace_back(vNodes[nDeparture]);
        currentNode = Next(nDeparture, vNodes);
        while(currentNode != nIntersect){
            polygonB.emplace_back(vNodes[currentNode]);
            currentNode = Next(currentNode, vNodes);
        }
        polygonB.emplace_back(vNodes[nIntersect]);
        polygonB.emplace_back(vfIntersect);

        GetRectangles(polygonA);
        GetRectangles(polygonB);
    }

    void Decompose(){
        rectangles.clear();
        GetRectangles(nodes);
    }

    bool InDirection(int nDir, olc::vf2d vfPos, int nNode, const std::vector<olc::vf2d>& vNodes){
        olc::vf2d rayPos;
        switch(nDir){
            case DIR_UP: { rayPos = olc::vf2d(vfPos.x,-INFINITY); break; }
            case DIR_DOWN: { rayPos = olc::vf2d(vfPos.x,INFINITY); break; }
            case DIR_LEFT: { rayPos = olc::vf2d(-INFINITY, vfPos.y); break; }
            case DIR_RIGHT: { rayPos = olc::vf2d(INFINITY, vfPos.y); break; }
        }
        int count = NodesIntersectCount(vfPos,rayPos);
        if((vfPos == vNodes[nNode] || vfPos == vNodes[Next(nNode, vNodes)]) && (nDir == DIR_RIGHT || nDir == DIR_DOWN)) return count % 2 == 1;
        return count % 2 == 0;
    }
    bool InDirection(int nDir, olc::vf2d vfPos, int nNode) { return InDirection(nDir, vfPos, nNode, nodes); }

    bool IsReflex(int nNode, const std::vector<olc::vf2d>& vNodes){
        int nodeDir = GetInDirection(nNode, vNodes);
        int prevNodeDir = GetInDirection(Prev(nNode, vNodes), vNodes);
        if(nodeDir == DIR_UP && prevNodeDir == DIR_RIGHT) return true;
        if(nodeDir == DIR_RIGHT && prevNodeDir == DIR_DOWN) return true;
        if(nodeDir == DIR_DOWN && prevNodeDir == DIR_LEFT) return true;
        if(nodeDir == DIR_LEFT && prevNodeDir == DIR_UP) return true;
        return false;
    }
    bool IsReflex(int nNode) { return IsReflex(nNode, nodes); }

    bool IsTargetInside(const olc::vf2d& vfTarget) const {
        olc::vf2d raycast = vfTarget + olc::vf2d(INFINITY,0);
        int count = NodesIntersectCount(vfTarget, raycast);
        return count % 2 == 0;
    }

    bool IsVertical(int node, const std::vector<olc::vf2d>& vNodes) { return vNodes[node].x == vNodes[Next(node, vNodes)].x; }
    bool IsVertical(int node) { return IsVertical(node, nodes); }
    bool IsVertical() { return IsVertical(currentNode); }

    int Next(int node, const std::vector<olc::vf2d>& vNodes) const {
        if(node == vNodes.size()-1) return 0;
        return node+1;
    }
    int Next(int node) { return Next(node, nodes); }

    int Prev(int node, const std::vector<olc::vf2d>& vNodes){
        if(node == 0) return vNodes.size()-1;
        return node-1;
    }
    int Prev(int node) { return Prev(node, nodes); }

    int NodesIntersect(olc::vf2d point1A, olc::vf2d point1B){
        olc::vf2d point2A = nodes[0];
        olc::vf2d point2B;
        for(int i=0; i < nodes.size(); i++){
            point2B = nodes[Next(i)];
            if(DoesIntersect(point1A, point1B, point2A, point2B))
                return i;
            point2A = point2B;
        }
        return -1;
    }

    int NodesIntersectCount(olc::vf2d point1A, olc::vf2d point1B, const std::vector<olc::vf2d>& vNodes) const {
        int count = 0;
        olc::vf2d point2A = vNodes[0];
        olc::vf2d point2B;
        for(int i=0; i < vNodes.size(); i++){
            point2B = vNodes[Next(i, vNodes)];
            if(DoesIntersect(point1A, point1B, point2A, point2B))
                count++;
            point2A = point2B;
        }
        return count;
    }
    int NodesIntersectCount(olc::vf2d point1A, olc::vf2d point1B) const { return NodesIntersectCount(point1A, point1B, nodes); }
    void DrawRectagles(olc::PixelGameEngine& pge, olc::Decal* decal, int layer, float fLeftMargin, float fTopMargin){
        float area = 0.0;
        if(rectangles.size() > 0){
            for(auto r : rectangles){
                if(DEBUG){
                    pge.SetDrawTarget(0, true);
                    pge.DrawRect(r.first, r.second-r.first, olc::MAGENTA);
                    pge.DrawLine(r.first,r.second, olc::MAGENTA);
                }
                pge.SetDrawTarget(layer);
                pge.DrawPartialDecal((olc::vf2d)r.first, (olc::vf2d)(r.second-r.first), decal, 
                                    (olc::vf2d)r.first-olc::vf2d(fLeftMargin,fTopMargin), (olc::vf2d)(r.second-r.first));
                area += (r.second.x-r.first.x) * (r.second.y-r.first.y);
            }
        }
        fAreaPercent = area / (290*190);
    }

    int GraftPath(int nDeparture, int nArrival, std::vector<olc::vf2d> vNewPath, const olc::vf2d& vfTarget){
        bool inverse = (nDeparture > nArrival) ||
                        (nDeparture == nArrival && std::abs(CalcDistance(nDeparture,vNewPath.back())) < std::abs(CalcDistance(nDeparture,vNewPath.front())));
        std::vector<olc::vf2d> vTempPath;
        int nNewCurrent = nDeparture+vNewPath.size();

        if(inverse) {
            int temp = nDeparture;
            nDeparture = nArrival;
            nArrival = temp;
            nNewCurrent = nDeparture;
            std::reverse(vNewPath.begin(), vNewPath.end());
        }

        for(int i = 0; i < nodes.size(); i++){
            if(i < nDeparture){
                vTempPath.emplace_back(nodes[i]);
            }
            else if(i == nDeparture){
                vTempPath.emplace_back(nodes[i]);
                for(int j = 0; j < vNewPath.size(); j++){
                    vTempPath.emplace_back(vNewPath[j]);
                }
            }
            else if(i > nArrival){
                vTempPath.emplace_back(nodes[i]);
            }
        }

        // Clearing unneeded nodes when departing from internal corners
        if(inverse){
            if(vNewPath.back() == nodes[nArrival] && AreColinear(nodes[Next(nArrival)],vNewPath.back(),vNewPath[vNewPath.size()-2]))
                vTempPath.erase(vTempPath.begin()+nDeparture+vNewPath.size());
            else if(vNewPath.back() == nodes[Next(nArrival)]){
                vTempPath.erase(vTempPath.begin()+nDeparture+vNewPath.size());
                if(!AreColinear(nodes[nArrival],vNewPath.back(),vNewPath[vNewPath.size()-2]))
                    vTempPath.erase(vTempPath.begin()+nDeparture+vNewPath.size());
            }
        }
        else{
            if(vNewPath[0] == nodes[Next(nDeparture)] && AreColinear(nodes[nDeparture],vNewPath[0],vNewPath[1])){
                vTempPath.erase(vTempPath.begin()+Next(nDeparture));
                nNewCurrent--;
            }
            else if(vNewPath[0] == nodes[nDeparture]){
                vTempPath.erase(vTempPath.begin()+nDeparture);
                nNewCurrent--;
                if(!AreColinear(nodes[Next(nDeparture)],vNewPath[0],vNewPath[1])){
                    vTempPath.erase(vTempPath.begin()+nDeparture);
                    nNewCurrent--;
                }
            }
        }

        // check if the target is inside
        int count = 0;
        olc::vf2d point1 = vTempPath[0];
        olc::vf2d point2;
        olc::vf2d raycast = vfTarget + olc::vf2d(INFINITY,0);
        for(int i=0; i < vTempPath.size(); i++){
            point2 = (i == vTempPath.size()-1) ? vTempPath[0] : vTempPath[i+1];
            if(DoesIntersect(vfTarget, raycast, point1, point2))
                count++;
            point1 = point2;
        }
        
        // target is not inside the temp path so it is discarded and the reverse path is created
        if(count % 2 == 0){
            vTempPath.clear();
            std::reverse(vNewPath.begin(), vNewPath.end()); 
            
            for(int i = 0; i < vNewPath.size(); i++){
                vTempPath.emplace_back(vNewPath[i]);
            }
            for(int i = nDeparture+1; i <= nArrival; i++){
                vTempPath.emplace_back(nodes[i]);
            }
            nNewCurrent = inverse ? vNewPath.size()-1 : vTempPath.size()-1;

            // Clearing unneeded nodes when departing from internal corners
            if(inverse){
                if(vNewPath[0] == nodes[Next(nArrival)] && AreColinear(nodes[nArrival],vNewPath[0],vNewPath[1])){
                    vTempPath.erase(vTempPath.begin());
                    nNewCurrent--;
                }
                else if(vNewPath[0] == nodes[nArrival]){
                    vTempPath.erase(vTempPath.begin());
                    nNewCurrent--;
                    if(!AreColinear(nodes[Next(nArrival)],vNewPath[0],vNewPath[1])){
                        vTempPath.erase(vTempPath.end());
                    }
                }
            }
            else{
                if(vNewPath.back() == nodes[nDeparture] && AreColinear(nodes[Next(nDeparture)],vNewPath.back(),vNewPath[vNewPath.size()-2])){
                    vTempPath.erase(vTempPath.begin()+vNewPath.size()-1);
                    nNewCurrent--;
                }
                else if(vNewPath.back() == nodes[Next(nDeparture)]){
                    vTempPath.erase(vTempPath.begin()+vNewPath.size()-1);
                    nNewCurrent--;
                    if(!AreColinear(nodes[nDeparture],vNewPath.back(),vNewPath[vNewPath.size()-2])){
                        vTempPath.erase(vTempPath.begin()+vNewPath.size()-1);
                        nNewCurrent--;
                    }
                }
            }

        }

        nodes = vTempPath;
        return nNewCurrent;
    }

};

#pragma region cShip
class cShip{
private:
    olc::vf2d pos;
    olc::vf2d lastPos;
    int direction;
    int lastDirection;
    float clampT;
    float clampB;
    float clampL;
    float clampR;

    bool snapToLine;
    float snapT;
    float snapB;
    float snapL;
    float snapR;

    std::vector<olc::vf2d> trail;

public:
    cShip(olc::vf2d vfPos, const float& fClampTop, const float& fClampBottom, const float& fClampLeft, const float& fClampRight): 
    pos(vfPos), lastPos(vfPos), clampT(fClampTop), clampB(fClampBottom), clampL(fClampLeft), clampR(fClampRight){
        direction = DIR_UP;
        lastDirection = DIR_UNDEFINED;
        snapToLine = false;
    }

    cShip(olc::vf2d vfPos): pos(vfPos){
        cShip(vfPos, 0, SCREEN_HEIGHT, 0, SCREEN_WIDTH);
    }

    void SetDirection(int nDirection) { direction = nDirection; }
    int GetDirection() { return direction; }

    void Move(int nDirection, float fSpeed){
        float top;
        float bottom;
        float left;
        float right;
        if(!snapToLine){
            top = clampT;
            bottom = clampB;
            left = clampL;
            right = clampR;
        }
        else{
            top = snapT;
            bottom = snapB;
            left = snapL;
            right = snapR;
        }

        switch(nDirection){
            case DIR_UP:{ pos.y -= fSpeed; break; }
            case DIR_DOWN:{ pos.y += fSpeed; break; }
            case DIR_LEFT:{ pos.x -= fSpeed; break; }
            case DIR_RIGHT:{ pos.x += fSpeed; break; }
        }

        if(pos.y < top) pos.y = top;
        else if(pos.y > bottom) pos.y = bottom;
        if(pos.x < left) pos.x = left;
        else if(pos.x > right) pos.x = right;

        // stopping the ship from going over the trail to not produce convex polygons
        if(!snapToLine && trail.size() > 1 && lastPos != trail.back()){
            for(int i = 0; i < trail.size() -1; i++){
                if((trail[i].x == trail[i+1].x && pos.y == lastPos.y) && 
                    ((pos.x <= trail[i].x && trail[i].x <= lastPos.x) || (lastPos.x <= trail[i].x && trail[i].x <= pos.x)) &&
                        ((trail[i].y <= pos.y && pos.y <= trail[i+1].y) || (trail[i+1].y <= pos.y && pos.y <= trail[i].y))){
                            if(pos!=trail.back()) pos = lastPos;
                        }
                else if((trail[i].y == trail[i+1].y && pos.x == lastPos.x) &&
                    ((pos.y <= trail[i].y && trail[i].y <= lastPos.y) || (lastPos.y <= trail[i].y && trail[i].y <= pos.y)) &&
                        ((trail[i].x <= pos.x && pos.x <= trail[i+1].x) || (trail[i+1].x <= pos.x && pos.x <= trail[i].x))){
                            if(pos!=trail.back()) pos = lastPos;
                        }
            }
        }
        //lastDirection = direction;
        direction = nDirection;
    }

    void SnapToLine(const olc::vf2d& start, const olc::vf2d& end, const bool& verticalLine){
        snapToLine = true;
        if(verticalLine){
            snapL = start.x;
            snapR = start.x;
            snapT = start.y < end.y ? start.y : end.y;
            snapB = start.y > end.y ? start.y : end.y;
            
            pos.x = start.x;
            if(pos.y < snapT) pos.y = snapT;
            if(pos.y > snapB) pos.y = snapB;
        }
        else{
            snapT = start.y;
            snapB = start.y;
            snapL = start.x < end.x ? start.x : end.x;
            snapR = start.x > end.x ? start.x : end.x;

            pos.y = start.y;
            if(pos.x < snapL) pos.x = snapL;
            if(pos.x > snapR) pos.x = snapR;
        }
    }

    void ReleaseFromLine(){
        snapToLine = false;
    }

    void Draw(olc::PixelGameEngine& pge){
        olc::vf2d vertA;
        olc::vf2d vertB;
        olc::vf2d vertC;
        int distR = 5;
        int distS = 2;
        int distT = 3;
        olc::Pixel color(olc::RED);

        switch(direction){
            case DIR_UP:{
                vertA = pos + olc::vf2d(0, -distR);
                vertB = pos + olc::vf2d(distT,distS);
                vertC = pos + olc::vf2d(-distT,distS);
                break;
            }
            case DIR_DOWN:{
                vertA = pos + olc::vf2d(0, distR);
                vertB = pos + olc::vf2d(distT,-distS);
                vertC = pos + olc::vf2d(-distT,-distS);
                break;
            }
            case DIR_LEFT:{
                vertA = pos + olc::vf2d(-distR,0);
                vertB = pos + olc::vf2d(distS,distT);
                vertC = pos + olc::vf2d(distS,-distT);
                break;

            }
            case DIR_RIGHT:{
                vertA = pos + olc::vf2d(distR,0);
                vertB = pos + olc::vf2d(-distS,distT);
                vertC = pos + olc::vf2d(-distS,-distT);
                break;
            }
        }

        pge.DrawLine(pos, pos, color);
        pge.DrawTriangle(vertA,vertB,vertC,color);
    }

    void SetLastPos(const olc::vf2d& vfLastPos) { lastPos = vfLastPos; }

    olc::vf2d GetPos() { return pos; }
    void SetPos(olc::vf2d vfPos) { pos = vfPos; }
    olc::vf2d GetLastPos() { return lastPos; }
    int GetLastDirection() { return lastDirection; }
    void SetLastDirection(int nDirection) { lastDirection = nDirection; }
    bool IsSnapd() { return snapToLine; }

    int AddTrail(olc::vf2d pos) { 
        trail.emplace_back(pos); 
        lastPos = pos;
        //lastDirection = direction;
        return trail.size();
    }
    void ClearTrail() { trail.clear(); }

    int DrawTrail(olc::PixelGameEngine& pge){
        if(!trail.empty()){
            pge.DrawLine(pos, trail.back(), olc::Pixel(255,255,0));
            for(int i=0; i < trail.size()-1; i++){
                pge.DrawLine(trail[i], trail[i+1], olc::Pixel(255,255,0));
                if(DEBUG) pge.DrawLine(trail[i], trail[i+1], olc::RED);
            }
        }
        return trail.size();
    }

    bool TrailSize() { return trail.size(); }

    std::vector<olc::vf2d> GetTrail(){ return trail; }

    bool DidTurn(int nDirection){
        if((lastDirection == DIR_UP || lastDirection == DIR_DOWN) && (nDirection == DIR_LEFT || nDirection == DIR_RIGHT)){
            return true;
        }
        if((lastDirection == DIR_LEFT || lastDirection == DIR_RIGHT) && (nDirection == DIR_UP || nDirection == DIR_DOWN)){
            return true;
        }
        return false;
    }

    bool DoesIntersectTrail(const olc::vf2d& vfBegin, const olc::vf2d& vfEnd, const sPath& path){
        if(trail.size() > 1){
            for(int i = 0; i < trail.size()-1; i++){
                if(path.DoesIntersect(trail[i],trail[i+1],vfBegin,vfEnd)) return true;
            }
        }
        if(path.DoesIntersect(trail.back(),pos,vfBegin,vfEnd)) return true;
        return false;
    }
};

#pragma region cEnemy
class cEnemy{
private:
    olc::Decal* decal;
    olc::vf2d position;
    olc::vf2d originalSize;
    olc::vf2d size;
    float scale;
    olc::Pixel color;
    olc::vf2d speed;
    float speedBase;

public:
    cEnemy(olc::Decal* dclImage, const olc::vf2d& vfPosition, const olc::vf2d& vfSize, const float& fScale, const float& fSpeed):
            decal(dclImage), position(vfPosition), originalSize(vfSize), scale(fScale), speedBase(fSpeed){
        size = originalSize * scale;
        color = olc::Pixel(distColor(rd),distColor(rd),distColor(rd));
        speed = olc::vf2d(speedBase,speedBase);
    }

    void Draw(olc::PixelGameEngine& pge){
        pge.SetDrawTarget(0,1);
        pge.DrawDecal(position, decal, olc::vf2d(scale,scale), color);
        if(DEBUG) pge.DrawRect(position, size, olc::CYAN);
    }

    olc::vf2d GetPos() { return position; }

    void Move(sPath& path, const float& fTime, cShip& ship){
        olc::vf2d lastPosition = position;
        position = position + speed*fTime;
 
        int intersectTopp = path.NodesIntersect(position,position+olc::vf2d(size.x,0));
        int intersectBott = path.NodesIntersect(position+olc::vf2d(0,size.y),position+olc::vf2d(size.x,size.y));
        int intersectLeft = path.NodesIntersect(position,position+olc::vf2d(0,size.y));
        int intersectRite = path.NodesIntersect(position+olc::vf2d(size.x,0),position+olc::vf2d(size.x,size.y));

        if(intersectTopp != -1){
            if(intersectBott != -1) speed.x *= -1;
            else if(intersectRite != -1) {
                if((position.x+size.x - path.nodes[intersectTopp].x) < (path.nodes[path.Next(intersectTopp)].y - position.y)) speed.x *= -1;
                else speed.y *= -1; 
            }
            else if(intersectLeft != -1) {
                if((path.nodes[intersectTopp].x - position.x) < (path.nodes[intersectTopp].y - position.y)) speed.x *= -1;
                else speed.y *= -1;
            }
            else {
                speed.y *= -1;
            }
        }
        else if(intersectBott != -1){
            if(intersectRite != -1){
                if((position.x+size.x - path.nodes[intersectBott].x) < (position.y+size.y - path.nodes[intersectBott].y)) speed.x *= -1; // problema
                else speed.y *= -1;
            }
            else if(intersectLeft != -1){
                if((path.nodes[intersectBott].x - position.x) < (position.y+size.y - path.nodes[path.Next(intersectBott)].y)) speed.x *= -1;
                else speed.y *= -1;
            }
            else speed.y *= -1;
        }
        else if(intersectLeft != -1){
            if(intersectRite != -1) speed.y *= -1;
            else speed.x *= -1;
        }
        else if(intersectRite != -1){
            speed.x *= -1;
        }

        if(intersectTopp != -1 || intersectBott != -1 || intersectLeft != -1 || intersectRite != -1){
            position = lastPosition;
            color = olc::Pixel(distColor(rd), distColor(rd), distColor(rd));

            float speedMod = distSpeed(rd);
            speed.x = speed.x < 0 ? -speedBase : speedBase;
            speed.y = speed.y < 0 ? -speedBase : speedBase;
            speed = speed * olc::vf2d(speedMod,2.0-speedMod);
        }

        if(!ship.IsSnapd() && (ship.DoesIntersectTrail(position, position+olc::vf2d(size.x,0), path) ||
                                ship.DoesIntersectTrail(position, position+olc::vf2d(0,size.y), path) ||
                                ship.DoesIntersectTrail(position+size, position+olc::vf2d(size.x,0), path) ||
                                ship.DoesIntersectTrail(position+size, position+olc::vf2d(0,size.y), path))){
            //ship destroy
            if(DEBUG) p2util::Echo("insersect", distColor(rd));
        }
    }

    void SetPos(olc::vf2d vfPos) { position = vfPos; }
};

struct sFont{
    olc::Decal* decal;
    std::string map; // List of characters in the font tileset in the order they appear
    olc::vi2d grid_size;
    olc::vi2d cell_size;

    sFont(olc::Decal* dclFont, olc::vi2d nGridSize, olc::vi2d nCellSize, std::string sMap):
        decal(dclFont), map(sMap), grid_size(nGridSize), cell_size(nCellSize) {}
    
    void Draw(olc::PixelGameEngine& pge, const std::string& text, olc::vf2d pos, const int length, 
                const olc::vi2d& anchor = olc::vi2d(0,0), const olc::Pixel tint = olc::WHITE){
        float width = length*cell_size.x;
        if(text.size() < length) width = text.size()*cell_size.x;
        const float height = cell_size.y;
    
        if(anchor.x == 1) pos.x -= width/2;
        else if(anchor.x == 2) pos.x -= width;
        if(anchor.y == 1) pos.y -= height/2;
        else if(anchor.y == 2) pos.y -= height;
    
        for(int f = 0; f < text.length() && f < length; f++){
            size_t charPos = map.find(text[f]);
            olc::vf2d letter = olc::vf2d(charPos%grid_size.x,charPos/grid_size.x)*cell_size;
            pge.DrawPartialDecal(pos+olc::vf2d(f*cell_size.x,0), decal, letter, cell_size, {1,1}, tint);
        }
    }
};

class GAME : public olc::PixelGameEngine{
#pragma region GAME_private
private:
    const float fFieldMarginLeft = 5;
    const float fFieldMarginRight = SCREEN_WIDTH - 5;
    const float fFieldMarginTop = 5;
    const float fFieldMarginBottom = SCREEN_HEIGHT - 15;

    cShip ship = cShip(olc::vf2d(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), fFieldMarginTop, fFieldMarginBottom, fFieldMarginLeft, fFieldMarginRight);
    sPath path;

    olc::vf2d vfCurrStart;
    olc::vf2d vfCurrEnd;
    int nCurrStrDir;
    int nCurrEndDir;

    int nDeparture;
    int nArrival;

    olc::vf2d vfTarget = olc::vf2d(SCREEN_WIDTH-20,20);

    olc::Sprite* sprBg_1;
    olc::Decal* dclBg_1;
    olc::Sprite* sprBg_2;
    olc::Decal* dclBg_2;

    int layerBg_1;
    int layerBg_2;

    olc::Sprite* sprEnemy;
    olc::Decal* dclEnemy;
    cEnemy* boss;

    olc::Sprite* sprFont;
    olc::Decal* dclFont;
    sFont* fontFFS;

public:
    GAME(){
        sAppName = "QuiX";
    }

    bool SnapShipToLine(int line){
        ship.SnapToLine(path.nodes[line], path.nodes[path.Next(line)], path.IsVertical(line));
        if (ship.GetPos() != olc::vf2d()) return true;
        else return false;
    }

    bool PathUpdate(int nNode){
        if(nNode < path.nodes.size()){
            vfCurrStart = path.nodes[nNode];
            vfCurrEnd = path.nodes[path.Next(nNode)];
            nCurrStrDir = path.GetStartExit(nNode);
            nCurrEndDir = path.GetEndExit(nNode);
            return true;
        }
        return false;
    }

    void ResetField(){
        int initialNode = 0;
        path.nodes.clear();
        path.nodes.emplace_back(olc::vf2d(fFieldMarginLeft,fFieldMarginTop));
        path.nodes.emplace_back(olc::vf2d(fFieldMarginRight,fFieldMarginTop));
        path.nodes.emplace_back(olc::vf2d(fFieldMarginRight,fFieldMarginBottom));
        path.nodes.emplace_back(olc::vf2d(fFieldMarginLeft,fFieldMarginBottom));

        path.rectangles.clear();
        path.rectangles.emplace_back(olc::vi2d(fFieldMarginLeft,fFieldMarginTop),olc::vi2d(fFieldMarginRight,fFieldMarginBottom));

        path.currentNode = 0;
        ship.SetDirection(DIR_UP);
        ship.SetLastDirection(DIR_UNDEFINED);
        SnapShipToLine(initialNode);
        PathUpdate(initialNode);
        path.currentNode = initialNode;
    }

    bool OnUserCreate(){
        sprBg_1 = new olc::Sprite("assets/ship_bg_day.png");
        dclBg_1 = new olc::Decal(sprBg_1);
        sprBg_2 = new olc::Sprite("assets/ship_bg_night.png");
        dclBg_2 = new olc::Decal(sprBg_2);
        sprFont = new olc::Sprite("assets/ffs_font_tile.png");
        dclFont = new olc::Decal(sprFont);
        sprEnemy = new olc::Sprite("assets/dvd_logo.png");
        dclEnemy = new olc::Decal(sprEnemy);

        boss = new cEnemy(dclEnemy, olc::vf2d(fFieldMarginRight-60, fFieldMarginTop+10), olc::vf2d(1000,441), 0.05, 40.0);

        fontFFS = new sFont(dclFont, olc::vi2d(16,6), olc::vi2d(8,8), 
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!?/:\"'-.,_;#+()%~*  =·>");

        ResetField();
        path.Decompose();

        Clear(olc::BLANK);
        layerBg_1 = CreateLayer();
        SetDrawTarget(layerBg_1);
        EnableLayer(layerBg_1, true);
        Clear(olc::BLANK);
        layerBg_2 = CreateLayer();
        SetDrawTarget(layerBg_2);
        EnableLayer(layerBg_2, true);
        Clear(olc::BLANK);
        
        return true;
    }

    #pragma region OnUserUpdate
    bool OnUserUpdate(float fElapsedTime){
        Clear(olc::BLANK);
        int direction = DIR_UNDEFINED;
        olc::vf2d lastPosSnapped = olc::vf2d();

        if(GetKey(olc::Key::UP).bHeld) direction = DIR_UP;
        else if(GetKey(olc::Key::DOWN).bHeld) direction = DIR_DOWN;
        else if(GetKey(olc::Key::LEFT).bHeld) direction = DIR_LEFT;
        else if(GetKey(olc::Key::RIGHT).bHeld) direction = DIR_RIGHT;

        if (direction != DIR_UNDEFINED){
            if(GetKey(olc::Key::SPACE).bPressed && ship.IsSnapd() && path.InDirection(direction, ship.GetPos(), path.currentNode)){
                ship.ReleaseFromLine(); // ship.snapedToLine = false;
                nDeparture = path.currentNode;
                path.currentNode = -1;
                ship.ClearTrail();
                ship.AddTrail(ship.GetPos()); // add departure coord
                lastPosSnapped = ship.GetPos();
            }

            if(ship.GetPos() == vfCurrEnd && direction == nCurrEndDir){
                path.currentNode = path.currentNode == path.nodes.size()-1 ? 0 : path.currentNode+1;
                PathUpdate(path.currentNode);
                SnapShipToLine(path.currentNode);
            }
            if(ship.GetPos() == vfCurrStart && direction == nCurrStrDir){
                path.currentNode = path.currentNode == 0 ? path.nodes.size()-1 : path.currentNode-1;
                PathUpdate(path.currentNode);
                SnapShipToLine(path.currentNode);
            }
            
            if(!ship.IsSnapd() && ship.DidTurn(direction)){
                 ship.AddTrail(ship.GetPos());
            }

            ship.Move(direction, fElapsedTime * 75);

            if(!ship.IsSnapd() && !GetKey(olc::Key::SPACE).bPressed){
                int line = path.NodesIntersect(ship.GetPos(),ship.GetLastPos());
                if(line != -1){
                    ship.SnapToLine(path.nodes[line], path.nodes[path.Next(line)], path.IsVertical(line)); // snap to old path to ensure AddTrail adds a valid pos for arr
                    if (nDeparture == line && ship.GetTrail().size() < 2){
                        path.currentNode = line;
                        PathUpdate(path.currentNode);
                    }
                    else{
                        ship.AddTrail(ship.GetPos()); // adding arr pos
                        path.currentNode = path.GraftPath(nDeparture, line, ship.GetTrail(), boss->GetPos());
                        PathUpdate(path.currentNode);
                        ship.SnapToLine(path.nodes[path.currentNode], path.nodes[path.Next(path.currentNode)], path.IsVertical(path.currentNode));
                        path.Decompose();
                    }
                }
            }
            ship.SetLastPos(ship.GetPos());
            ship.SetLastDirection(ship.GetDirection());
        }

        boss->Move(path, fElapsedTime, ship);

        //test
        if(GetKey(olc::Key::R).bPressed) {
            ResetField();
            boss->SetPos(olc::vf2d(10,100));
        }
        if(GetKey(olc::Key::ESCAPE).bPressed) return false;
        
        SetDrawTarget(layerBg_2);
        DrawDecal(olc::vf2d(fFieldMarginLeft,fFieldMarginTop), dclBg_2);

        path.DrawRectagles(*this, dclBg_1, layerBg_1, fFieldMarginLeft, fFieldMarginTop);

        SetDrawTarget(0, true);
        path.DrawAll(*this, olc::Pixel(192,192,192));
        if(DEBUG && path.currentNode != -1) path.Draw(*this, path.currentNode, olc::GREEN);
        if(DEBUG) DrawLine(path.nodes[0], path.nodes[0], olc::BLUE);
        ship.Draw(*this);
        // DrawLine(vfTarget, vfTarget, olc::RED);
        // DrawRect(vfTarget-3, olc::vi2d(6,6), olc::RED);
        boss->Draw(*this);
        if(!ship.IsSnapd()) ship.DrawTrail(*this);

        fontFFS->Draw(*this, "Remaining:", {fFieldMarginLeft,fFieldMarginBottom+3}, 11);
        fontFFS->Draw(*this, std::to_string(path.fAreaPercent*100), {10*8+fFieldMarginLeft,fFieldMarginBottom+3}, 5);
        fontFFS->Draw(*this, "%", {15*8+fFieldMarginLeft,fFieldMarginBottom+3}, 1);
        return true;
    }
};

int main(){
    GAME game;
    if(game.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_SIZE, PIXEL_SIZE)) game.Start();
    else return -10;
    return 0;
}