#define OLC_PGE_APPLICATION
#include "OneLoneCoder/olcPixelGameEngine.h"

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

    bool IsVertical(int node) { return node % 2 == 0; }
    bool IsVertical() { return currentNode % 2 == 0;}

    olc::vf2d GetEnd(int node) { 
        if(node < nodes.size())
            return IsVertical(node) ? olc::vf2d(nodes[node],0) : olc::vf2d(0,nodes[node]); 
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
        else if(node == nodes.size()){
            return GetEndAbs(node);
        }
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
        if(node == nodes.size()){
            return origin;
        }
        else
            return olc::vf2d();
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

public:
    cShip(olc::vf2d vfPos): pos(vfPos){
        direction = DIR_UP;
    }

    void SetDirection(int nDirection) { direction = nDirection; }

    void Move(int nDirection, float fSpeed){
        switch(nDirection){
            case DIR_UP:{
                pos.y -= fSpeed;
                break;
            }
            case DIR_DOWN:{
                pos.y += fSpeed;
                break;
            }
            case DIR_LEFT:{
                pos.x -= fSpeed;
                break;
            }
            case DIR_RIGHT:{
                pos.x += fSpeed;
                break;
            }
        }
        direction = nDirection;
    }

    void MoveConstraing(){

    }

    void SnapToLine(){

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
};

class GAME : public olc::PixelGameEngine{
private:
    cShip ship = cShip(olc::vf2d(ScreenWidth()/2, ScreenHeight()/2));
    sPath path = sPath(olc::vf2d(5,5));

public:
    GAME(){
        sAppName = "QuiX";
    }

    bool OnUserCreate(){
        path.nodes.push_back(ScreenWidth()-10);
        path.nodes.push_back(ScreenHeight()-10);
        path.nodes.push_back(-ScreenWidth()+10);
        return true;
    }

    bool OnUserUpdate(float fElapsedTime){
        int direction = DIR_UNDEFINED;

        if(GetKey(olc::Key::UP).bHeld) direction = DIR_UP;
        else if(GetKey(olc::Key::DOWN).bHeld) direction = DIR_DOWN;
        else if(GetKey(olc::Key::LEFT).bHeld) direction = DIR_LEFT;
        else if(GetKey(olc::Key::RIGHT).bHeld) direction = DIR_RIGHT;

        if (direction != DIR_UNDEFINED){
            ship.Move(direction, fElapsedTime * 75);
        }

        ship.Draw(*this);
        path.DrawAll(*this);
        path.Draw(*this, 3, olc::GREEN);
        return true;
    }
};

int main(){
    GAME game;
    if(game.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_SIZE, PIXEL_SIZE)) game.Start();
    else return -1;
    return 0;
}