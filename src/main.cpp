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
        // else if(node == nodes.size()){
        //     return GetEndAbs(node);
        // }
        else 
            return olc::vf2d();
    }

    olc::vf2d GetEndAbs(int node){
        if(node < nodes.size()){
            olc::vf2d coord = origin;
            for(int i = 0; i <= node; i++){
                coord = coord + GetEnd(i);
            }
            return coord;
        }
        // if(node == nodes.size()){
        //     return origin;
        // }
        else
            return olc::vf2d();
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

    }

    int GetEndExit(int node){
        
    }

    void Draw(olc::PixelGameEngine& pge, int node, olc::Pixel color = olc::WHITE){
        pge.DrawLine(GetStartAbs(node), GetEndAbs(node), color);
    }

    void DrawAll(olc::PixelGameEngine& pge, olc::Pixel color = olc::WHITE){
        for(int i = 0; i <= nodes.size(); i++){
            Draw(pge, i, color);
        }
    }

};

class cShip{
private:
    olc::vf2d pos;
    int direction;
    float clampT;
    float clampB;
    float clampL;
    float clampR;

    bool snapToLine;
    float snapT;
    float snapB;
    float snapL;
    float snapR;

public:
    cShip(olc::vf2d vfPos, const float& fClampTop, const float& fClampBottom, const float& fClampLeft, const float& fClampRight): 
    pos(vfPos), clampT(fClampTop), clampB(fClampBottom), clampL(fClampLeft), clampR(fClampRight){
        direction = DIR_UP;
        snapToLine = false;
    }

    cShip(olc::vf2d vfPos): pos(vfPos){
        cShip(vfPos, 0, SCREEN_HEIGHT, 0, SCREEN_WIDTH);
    }

    void SetDirection(int nDirection) { direction = nDirection; }

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

    olc::vf2d GetPos() { return pos; }
};

class GAME : public olc::PixelGameEngine{
private:
    const float fFieldMarginLeft = 5;
    const float fFieldMarginRight = SCREEN_WIDTH - 5;
    const float fFieldMarginTop = 5;
    const float fFieldMarginBottom = SCREEN_HEIGHT - 5;

    cShip ship = cShip(olc::vf2d(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), fFieldMarginTop, fFieldMarginBottom, fFieldMarginLeft, fFieldMarginRight);
    sPath path = sPath(olc::vf2d(fFieldMarginLeft, fFieldMarginTop));

public:
    GAME(){
        sAppName = "QuiX";
    }

    bool SnapShipToLine(int line){
        ship.SnapToLine(path.GetStartAbs(line), path.GetEndAbs(line), path.IsVertical(line));
        if (ship.GetPos() != olc::vf2d()) return true;
        else return false;
    }

    bool OnUserCreate(){
        path.nodes.push_back(fFieldMarginRight-fFieldMarginLeft);
        path.nodes.push_back(fFieldMarginBottom-fFieldMarginTop);
        path.nodes.push_back(-fFieldMarginRight+fFieldMarginLeft);
        path.nodes.push_back(-fFieldMarginBottom+fFieldMarginTop);

        SnapShipToLine(0);
        return true;
    }

    bool OnUserUpdate(float fElapsedTime){
        Clear(olc::BLANK);
        int direction = DIR_UNDEFINED;

        if(GetKey(olc::Key::UP).bHeld) direction = DIR_UP;
        else if(GetKey(olc::Key::DOWN).bHeld) direction = DIR_DOWN;
        else if(GetKey(olc::Key::LEFT).bHeld) direction = DIR_LEFT;
        else if(GetKey(olc::Key::RIGHT).bHeld) direction = DIR_RIGHT;

        if (direction != DIR_UNDEFINED){
            if(GetKey(olc::Key::SPACE).bPressed)
                ship.ReleaseFromLine();
            ship.Move(direction, fElapsedTime * 75);
        }

        path.DrawAll(*this);
        path.Draw(*this, path.currentNode, olc::GREEN);
        ship.Draw(*this);
        return true;
    }
};

int main(){
    GAME game;
    if(game.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_SIZE, PIXEL_SIZE)) game.Start();
    else return -1;
    return 0;
}