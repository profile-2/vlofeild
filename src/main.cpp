#define OLC_PGE_APPLICATION
#include "OneLoneCoder/olcPixelGameEngine.h"
#include "profile_2/p2util.h"

#define SCREEN_WIDTH    300
#define SCREEN_HEIGHT   200
#define PIXEL_SIZE      4

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
+checking target and reverse graft
    doesn't work for vertical departure node
departure or arrival path is colinear with old path
prevent the newpath to intersect with itself, iterate over pairs of trail points and test intersect with pos-lastpos, if true revert to lastpos

bugs:
sometimes when cutting inverted ship snaps to the wrong line. Happends when cutting up or right without turning once

idea:
recursive division of areas to check inner area, checking vertices with raytracing
*/

#pragma region sPath
struct sPath{
    std::vector<float> nodes;
    int currentNode;
    olc::vf2d origin;

    sPath(olc::vf2d originPos): origin(originPos){
        currentNode = 0;
    }

    bool IsVertical(int node) { return node % 2; }
    bool IsVertical() { return currentNode % 2; }

    olc::vf2d GetEnd(int node) { 
        if(node < nodes.size())
            return IsVertical(node) ? olc::vf2d(0,nodes[node]) : olc::vf2d(nodes[node],0); 
        else
            return olc::vf2d();
    }

    olc::vf2d GetEnd() { return GetEnd(currentNode);}

    olc::vf2d GetEnd2(const std::vector<float>& vNodes, int node){
        if(node < vNodes.size()){
            return IsVertical(node) ? olc::vf2d(0,vNodes[node]) : olc::vf2d(vNodes[node],0);
        }
        else
            return olc::vf2d();
    }

    void AddNode(std::vector<float>& vNodes, olc::vf2d vfPointA, olc::vf2d vfPointB){ // not used?
        bool vertical = vfPointA.x == vfPointB.x;
        // if(vertical && vNodes.empty()) vNodes.push_back(0);
        if(vertical) vNodes.push_back(vfPointB.y - vfPointA.y);
        else vNodes.push_back(vfPointB.x - vfPointA.x);
    }

    void AddNode(olc::vf2d vfPointA, olc::vf2d vfPointB){ // not used?
        bool vertical = vfPointA.x == vfPointB.x;
        if(vertical && nodes.empty()) nodes.push_back(0);
        if(vertical) nodes.push_back(vfPointB.y - vfPointA.y);
        else nodes.push_back(vfPointB.x - vfPointA.x);
    }

    float CalcPath(olc::vf2d vfPointA, olc::vf2d vfPointB){
        bool vertical = vfPointA.x == vfPointB.x;
        if(vertical) return (vfPointB.y - vfPointA.y);
        else return (vfPointB.x - vfPointA.x);
    }


    float ModEnd(int nNode, const olc::vf2d& vfNewEnd){ 
        if(nNode < nodes.size()){
            olc::vf2d start = GetStartAbs(nNode);
            if(IsVertical(nNode)) return vfNewEnd.y - start.y;
            else return vfNewEnd.x - start.x;
        }
        return 0;
    }

    float ModStart(int nNode, const olc::vf2d& vfNewStart){
        if(nNode < nodes.size()){
            olc::vf2d end = GetEndAbs(nNode);
            if(IsVertical(nNode)) return end.y - vfNewStart.y;
            else return end.x - vfNewStart.x;
        }
        return 0;
    }

    olc::vf2d GetStartAbs(int node){
        if(node == 0){
            return origin;
        }
        else if(node < nodes.size()){
            olc::vf2d coord = origin;
            for(int i = 0; i < node; i++){
                coord = coord + GetEnd(i);
            }
            return coord;
        }
        else 
            return olc::vf2d();
    }
    olc::vf2d GetStartAbs(){
        return GetStartAbs(currentNode);
    }

    olc::vf2d GetEndAbs(int node){
        if(node < nodes.size()){
            olc::vf2d coord = origin;
            for(int i = 0; i <= node; i++){
                coord = coord + GetEnd(i);
            }
            return coord;
        }
        else
            return olc::vf2d();
    }
    olc::vf2d GetEndAbs(){
        return GetEndAbs(currentNode);
    }

    void GetPoints(std::vector<olc::vf2d>& ends, const int& node){
        ends.clear();
        ends.emplace_back(GetStartAbs(node));
        ends.emplace_back(GetEndAbs(node));
    }
    void GetPoints(std::vector<olc::vf2d>& ends){
        GetPoints(ends, currentNode);
    }

    int GetStartExit(int node){
        int prevNode = node == 0 ? nodes.size()-1 : node-1;

        if(IsVertical(node)){
            if (nodes[prevNode] < 0)
                return DIR_RIGHT;
            else
                return DIR_LEFT;
        }
        else{
            if (nodes[prevNode] < 0)
                return DIR_DOWN;
            else
                return DIR_UP;
        }
        return DIR_UNDEFINED;
    }

    int GetEndExit(int node){
        int nextNode = node == nodes.size()-1 ? 0 : node+1;
        if(IsVertical(node)){
            if (nodes[nextNode] < 0)
                return DIR_LEFT;
            else
                return DIR_RIGHT;
        }
        else{
            if (nodes[nextNode] < 0)
                return DIR_UP;
            else
                return DIR_DOWN;
        }
        return DIR_UNDEFINED;
    }

    bool DoesIntersect(int node, olc::vf2d point1A, olc::vf2d point1B, olc::vf2d point2A, olc::vf2d point2B){
        
        float point2AX = point2A.x < point2B.x ? point2A.x : point2B.x;
        float point2BX = point2A.x > point2B.x ? point2A.x : point2B.x;
        float point2AY = point2A.y < point2B.y ? point2A.y : point2B.y;
        float point2BY = point2A.y > point2B.y ? point2A.y : point2B.y;

        float point1AX = point1A.x < point1B.x ? point1A.x : point1B.x;
        float point1BX = point1A.x > point1B.x ? point1A.x : point1B.x;
        float point1AY = point1A.y < point1B.y ? point1A.y : point1B.y;
        float point1BY = point1A.y > point1B.y ? point1A.y : point1B.y;

        if(IsVertical(node)){
            if(point1AX <= point2AX && point2AX <= point1BX && point2AY <= point1AY && point1AY <= point2BY)
                return true;
        }
        else{
            if(point1AY <= point2AY && point2AY <= point1BY && point2AX <= point1AX && point1AX <= point2BX)
                return true;
        }
        return false;
    }
    bool DoesIntersect(int node, olc::vf2d point1A, olc::vf2d point1B){
        return DoesIntersect(node, point1A, point1B, GetStartAbs(node), GetEndAbs(node));
    }

    int NodesIntersect(olc::vf2d point1A, olc::vf2d point1B){
        olc::vf2d point2A = origin;
        olc::vf2d point2B;
        for(int i=0; i < nodes.size(); i++){
            point2B = GetEnd(i) + point2A;
            if(DoesIntersect(i, point1A, point1B, point2A, point2B))
                return i;
            point2A = point2B;
        }
        return -1;
    }

    int NodesIntersectCount(olc::vf2d point1A, olc::vf2d point1B){
        int count = 0;
        olc::vf2d point2A = origin;
        olc::vf2d point2B;
        for(int i=0; i < nodes.size(); i++){
            point2B = GetEnd(i) + point2A;
            if(DoesIntersect(i, point1A, point1B, point2A, point2B))
                count++;
            point2A = point2B;
        }
        return count;
    }

    // int NodesIntersectCount(const std::vector<float>& vNodes, olc::vf2d point1A, olc::vf2d point1B){
    //     int count = 0;
    //     olc::vf2d point2A = origin;
    //     olc::vf2d point2B;
    //     for(int i=0; i < nodes.size(); i++){
    //         point2B = GetEnd(i) + point2A;
    //         if(DoesIntersect(i, point1A, point1B, point2A, point2B))
    //             count++;
    //         point2A = point2B;
    //     }
    //     return count;
    // }

    void Draw(olc::PixelGameEngine& pge, int node, olc::Pixel color = olc::WHITE){
        pge.DrawLine(GetStartAbs(node), GetEndAbs(node), color);
    }

    void DrawAll(olc::PixelGameEngine& pge, olc::Pixel color = olc::WHITE){
        for(int i = 0; i <= nodes.size(); i++){
            Draw(pge, i, color);
        }
    }

    bool InDirection(int nDir, olc::vf2d vfPos){
        olc::vf2d rayPos;
        switch(nDir){
            case DIR_UP: { rayPos = olc::vf2d(vfPos.x,-SCREEN_WIDTH); break; } // SCREEN_WIDTH is just an arbitrary distance outside the screen
            case DIR_DOWN: { rayPos = olc::vf2d(vfPos.x,SCREEN_WIDTH); break; }
            case DIR_LEFT: { rayPos = olc::vf2d(-SCREEN_WIDTH, vfPos.y); break; }
            case DIR_RIGHT: { rayPos = olc::vf2d(SCREEN_WIDTH, vfPos.y); break; }
        }
        int count = NodesIntersectCount(vfPos,rayPos);
        return count % 2 == 0;
    }

    bool IsTargetInside(const olc::vf2d& vfTarget){
        olc::vf2d raycast = vfTarget + olc::vf2d(SCREEN_WIDTH,0);
        int count = NodesIntersectCount(vfTarget, raycast);
        p2util::Echo(count);
        return count % 2 == 0;
    }

    int GraftPath(int nDeparture, int nArrival, const std::vector<olc::vf2d>& vNewPath, const olc::vf2d& vfTarget){
        bool inverse = (nDeparture > nArrival) || (nDeparture == nArrival && CalcPath(GetEndAbs(nDeparture),vNewPath.back()) < CalcPath(GetEndAbs(nDeparture),vNewPath[0]));
        if (inverse){
            int temp = nArrival;
            nArrival = nDeparture;
            nDeparture = temp;
        }
        std::vector<float> vTempPath;
        float departureEnd = ModEnd(nDeparture, inverse ? vNewPath.back() : vNewPath.front());
        float arrivalStart = ModStart(nArrival, inverse ? vNewPath.front() : vNewPath.back());

        for(int i = 0; i < nodes.size(); i++){
            if(i < nDeparture){
                vTempPath.push_back(nodes[i]);
            }
            else if (i == nDeparture){
                vTempPath.push_back(departureEnd);

                int iterB = inverse ? vNewPath.size()-1 : 0; //this is silly
                int iterE = inverse ? 0 : vNewPath.size()-1; //this is silly
                int iterMod = inverse ? -1 : 1;             //this is silly
                for(int j = iterB; j != iterE; j+=iterMod){
                    vTempPath.push_back(CalcPath(vNewPath[j], vNewPath[j+iterMod]));
                }

                if(nDeparture == nArrival){
                    vTempPath.push_back(arrivalStart);
                }
            }
            else if (i == nArrival){
                vTempPath.push_back(arrivalStart);
            }
            else if (i > nArrival){
                vTempPath.push_back(nodes[i]);
            }
        }

        //check if target is inside
        int count = 0;
        olc::vf2d point2A = origin;
        olc::vf2d point2B;
        olc::vf2d raycast = vfTarget + olc::vf2d(SCREEN_WIDTH,0);
        for(int i=0; i < vTempPath.size(); i++){
            point2B = GetEnd2(vTempPath, i) + point2A;
            if(DoesIntersect(i, vfTarget, raycast, point2A, point2B))
                count++;
            point2A = point2B;
        }

        if(count % 2){
            nodes = vTempPath;
        }
        else{
            vTempPath.clear();
            
            arrivalStart = ModStart(nDeparture, inverse ? vNewPath.back() : vNewPath.front());
            departureEnd = ModEnd(nArrival, inverse ? vNewPath.front() : vNewPath.back());
            
            if(nArrival == nDeparture){
                if(!inverse)
                    arrivalStart = ModEnd(nArrival, vNewPath.back()) - ModEnd(nArrival, vNewPath.front());
                else
                    arrivalStart = ModEnd(nArrival, vNewPath.front()) - ModEnd(nArrival, vNewPath.back());
            }

            vTempPath.emplace_back(arrivalStart);
            
            if(nArrival!=nDeparture){
                int oldNodes = nArrival - nDeparture - 1;
                if(oldNodes > 0){
                    for(int i = 0; i < oldNodes; i++){
                        vTempPath.emplace_back(i+nDeparture+1);
                    }
                }
                
                vTempPath.emplace_back(departureEnd);
            }
            
            int iterB = inverse ? 0 : vNewPath.size()-1; // this is silly
            int iterE = inverse ? vNewPath.size()-1 : 0; // this is silly
            int iterMod = inverse ? 1 : -1; // this is silly
            for(int i = iterB; i != iterE; i+=iterMod){
                vTempPath.push_back(CalcPath(vNewPath[i], vNewPath[i+iterMod])); 
            }
            
            origin = inverse ? vNewPath.back() : vNewPath.front();
            nodes = vTempPath;

            if(inverse) return nDeparture;
            return nArrival-nDeparture;
        }
        
        if(inverse) return nArrival;
        return nDeparture + vNewPath.size();
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
        // p2util::Echo(pos);
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
                pge.DrawLine(trail[i], trail[i+1], olc::RED);
            }
        }
        return trail.size();
    }

    bool TrailSize() { p2util::Echo(trail.size()); return trail.size(); }

    std::vector<olc::vf2d> GetTrail(){ return trail; }

    bool DidTurn(int nDirection){
        if((lastDirection == DIR_UP || lastDirection == DIR_DOWN) && (nDirection == DIR_LEFT || nDirection == DIR_RIGHT)){
            // p2util::Echo("turn1");    
            return true;
        }
        if((lastDirection == DIR_LEFT || lastDirection == DIR_RIGHT) && (nDirection == DIR_UP || nDirection == DIR_DOWN)){
            // p2util::Echo("turn2");    
            return true;
        }
        return false;
    }
};

class GAME : public olc::PixelGameEngine{
#pragma region GAME_private
private:
    const float fFieldMarginLeft = 5;
    const float fFieldMarginRight = SCREEN_WIDTH - 5;
    const float fFieldMarginTop = 5;
    const float fFieldMarginBottom = SCREEN_HEIGHT - 5;

    cShip ship = cShip(olc::vf2d(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), fFieldMarginTop, fFieldMarginBottom, fFieldMarginLeft, fFieldMarginRight);
    sPath path = sPath(olc::vf2d(fFieldMarginLeft, fFieldMarginTop));
    //sPath trail = sPath(olc::vf2d());

    olc::vf2d vfCurrStart;
    olc::vf2d vfCurrEnd;
    int nCurrStrDir;
    int nCurrEndDir;

    int nDeparture;
    int nArrival;

    olc::vf2d vfTarget = olc::vf2d(SCREEN_WIDTH-20,20);

public:
    GAME(){
        sAppName = "QuiX";
    }

    bool SnapShipToLine(int line){
        ship.SnapToLine(path.GetStartAbs(line), path.GetEndAbs(line), path.IsVertical(line));
        if (ship.GetPos() != olc::vf2d()) return true;
        else return false;
    }

    bool PathUpdate(int nNode){
        if(nNode < path.nodes.size()){
            vfCurrStart = path.GetStartAbs(nNode);
            vfCurrEnd = path.GetEndAbs(nNode);
            nCurrStrDir = path.GetStartExit(nNode);
            nCurrEndDir = path.GetEndExit(nNode);
            return true;
        }
        return false;
    }

    bool OnUserCreate(){
        path.nodes.push_back(fFieldMarginRight-fFieldMarginLeft);
        path.nodes.push_back(fFieldMarginBottom-fFieldMarginTop-40);
        path.nodes.push_back(-20);
        path.nodes.push_back(20);
        path.nodes.push_back(20);
        path.nodes.push_back(20);
        path.nodes.push_back(-fFieldMarginRight+fFieldMarginLeft+40);
        path.nodes.push_back(-20);
        path.nodes.push_back(-20);
        path.nodes.push_back(20);
        path.nodes.push_back(-20);
        path.nodes.push_back(-fFieldMarginBottom+fFieldMarginTop);

        //test
        // p2util::Echo(path.DoesIntersect(0, olc::vf2d(0,0), olc::vf2d(0,10), olc::vf2d(-5,5), olc::vf2d(5,5)));
        // p2util::Echo(path.DoesIntersect(1, olc::vf2d(-5,5), olc::vf2d(5,5), olc::vf2d(0,0), olc::vf2d(0,10)));
        // p2util::Echo(path.DoesIntersect(0, olc::vf2d(0,0), olc::vf2d(0,0), olc::vf2d(0.0000,0), olc::vf2d(0.0000,0)));

        SnapShipToLine(0);
        PathUpdate(0);
        return true;
    }

    #pragma region OnUserUpdate
    bool OnUserUpdate(float fElapsedTime){
        Clear(olc::BLANK);
        int direction = DIR_UNDEFINED;

        if(GetKey(olc::Key::UP).bHeld) direction = DIR_UP;
        else if(GetKey(olc::Key::DOWN).bHeld) direction = DIR_DOWN;
        else if(GetKey(olc::Key::LEFT).bHeld) direction = DIR_LEFT;
        else if(GetKey(olc::Key::RIGHT).bHeld) direction = DIR_RIGHT;

        if (direction != DIR_UNDEFINED){
            if(GetKey(olc::Key::SPACE).bPressed && ship.IsSnapd() && path.InDirection(direction, ship.GetPos())){
                ship.ReleaseFromLine(); // ship.snapedToLine = false;
                nDeparture = path.currentNode;
                path.currentNode = -1;
                ship.ClearTrail();
                ship.AddTrail(ship.GetPos()); // add departure coord
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
                //ship.SetPos(olc::vf2d((int)ship.GetPos().x,(int)ship.GetPos().y));
                 ship.AddTrail(ship.GetPos());
            }

            ship.Move(direction, fElapsedTime * 75);

            if(!ship.IsSnapd() && !GetKey(olc::Key::SPACE).bHeld){
                int line = path.NodesIntersect(ship.GetPos(),ship.GetLastPos());
                if(line != -1){
                    ship.SnapToLine(path.GetStartAbs(line), path.GetEndAbs(line), path.IsVertical(line)); // snap to old path to ensure AddTrail adds a valid pos for arr
                    if (nDeparture == line && ship.GetTrail().size() < 2){
                        p2util::Echo(ship.GetTrail().size());
                        path.currentNode = line;
                        PathUpdate(path.currentNode);
                    }
                    else{
                        ship.AddTrail(ship.GetPos()); // adding arr pos
                        path.currentNode = path.GraftPath(nDeparture, line, ship.GetTrail(), vfTarget); // arr is line, graftPath return new arr node
                        PathUpdate(path.currentNode);
                        ship.SnapToLine(path.GetStartAbs(), path.GetEndAbs(), path.IsVertical());
                    }
                }
            }
            ship.SetLastPos(ship.GetPos());
            ship.SetLastDirection(ship.GetDirection());
        }

        //test

        path.DrawAll(*this);
        path.Draw(*this, path.currentNode, olc::GREEN);
        ship.Draw(*this);
        DrawLine(vfTarget, vfTarget, olc::RED);
        if(!ship.IsSnapd()) ship.DrawTrail(*this);
        return true;
    }
};

int main(){
    GAME game;
    if(game.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_SIZE, PIXEL_SIZE)) game.Start();
    else return -1;
    return 0;
}