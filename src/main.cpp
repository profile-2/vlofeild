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

struct sPath {
    olc::vf2d start;
    olc::vf2d end;
    char direction;
    
    sPath(olc::vf2d vfStart, olc::vf2d vfEnd):
            start(vfStart), end(vfEnd){
        if(vfStart.x == vfEnd.x) direction = DIR_VERTICAL;
        else if (vfStart.y == vfEnd.y) direction = DIR_HORIZONTAL;
        else {
            start = olc::vf2d();
            end = olc::vf2d();
            direction = DIR_UNDEFINED;
        }
    }

    bool DoesIntersect(const olc::vf2d& posA, const olc::vf2d& posB){
        if (direction == DIR_HORIZONTAL && posA.x == posB.x){
            if(((start.x <= posA.x && posA.x <= end.x) || (end.x <= posA.x && posA.x <= start.x)) && 
               ((posA.y <= start.y && start.y <= posB.y) || (posB.y <= start.y && start.y <= posB.y)))
                    return true;
        }
        else if (direction == DIR_VERTICAL && posA.y == posB.y){
            if(((start.y <= posA.y && posA.y <= end.y) || (end.y <= posA.y && posA.y <= start.y)) && 
               ((posA.x <= start.x && start.x <= posB.x) || (posB.x <= start.x && start.x <= posB.x)))
                    return true;
        }
        return false;
    }
};

class GAME : public olc::PixelGameEngine {
private:
    const int nCorner = 5;

    int nPosR = 5;
    int nPosS = 3;
    int nPosT = 2;
    olc::vf2d vfPos = olc::vf2d(ScreenWidth()/2, ScreenHeight() - nCorner);
    olc::vi2d viPosA = vfPos + olc::vi2d(0, -nPosR);
    olc::vi2d viPosB = vfPos + olc::vi2d(nPosS, nPosT);
    olc::vi2d viPosC = vfPos + olc::vi2d(-nPosS, nPosT);

    olc::vf2d vfPosPrev;
    std::vector<olc::vf2d> vNewNodes;
    bool bShipFree = false;
    char nLastDir = DIR_UNDEFINED;

    std::vector<sPath> vPath;
    olc::vf2d vfTarget = olc::vf2d(10.0,10.0);
    int nCurrPath = 2;
    int nPathPrev = -1;

public:
    GAME() {
        sAppName = "VLOFEILD!";
    }

    char GetDirection(const float& fElapsedTime){
        if(GetKey(olc::Key::RIGHT).bHeld){
            
            viPosA = vfPos + olc::vi2d(nPosR,0);
            viPosB = vfPos + olc::vi2d(-nPosT,nPosS);
            viPosC = vfPos + olc::vi2d(-nPosT,-nPosS);
            return DIR_RIGHT;
        }
        else if(GetKey(olc::Key::LEFT).bHeld){
            
            viPosA = vfPos + olc::vi2d(-nPosR,0);
            viPosB = vfPos + olc::vi2d(nPosT,nPosS);
            viPosC = vfPos + olc::vi2d(nPosT,-nPosS);
            return DIR_LEFT;
        }
        else if(GetKey(olc::Key::UP).bHeld){
            
            viPosA = vfPos + olc::vi2d(0, -nPosR);
            viPosB = vfPos + olc::vi2d(nPosS, nPosT);
            viPosC = vfPos + olc::vi2d(-nPosS, nPosT);
            return DIR_UP;
        }
        else if(GetKey(olc::Key::DOWN).bHeld){
            
            viPosA = vfPos + olc::vi2d(0, nPosR);
            viPosB = vfPos + olc::vi2d(nPosS, -nPosT);
            viPosC = vfPos + olc::vi2d(-nPosS, -nPosT);
            return DIR_DOWN;
        }
        return DIR_UNDEFINED;
    }

    char GetOutDir(const std::vector<sPath>& vPath, int nCurrPath, char outPoint = DIR_START){
        
        if(outPoint = DIR_START){
            int nPrevPath = (nCurrPath == 0 ? (vPath.size()-1) : (nCurrPath-1));

            if(vPath[nPrevPath].start.x < vPath[nPrevPath].end.x) return DIR_LEFT;
            else if(vPath[nPrevPath].start.x > vPath[nPrevPath].end.x) return DIR_RIGHT;
            else if(vPath[nPrevPath].start.y < vPath[nPrevPath].end.y) return DIR_UP;
            else if(vPath[nPrevPath].start.y > vPath[nPrevPath].end.y) return DIR_DOWN;
            else return DIR_UNDEFINED;
        }
        else if(outPoint = DIR_END){
            int nNextPath = (nCurrPath == (vPath.size()-1) ? 0 : (nCurrPath+1));

            if(vPath[nNextPath].start.x < vPath[nNextPath].end.x) return DIR_RIGHT;
            else if(vPath[nNextPath].start.x > vPath[nNextPath].end.x) return DIR_LEFT;
            else if(vPath[nNextPath].start.y < vPath[nNextPath].end.y) return DIR_DOWN;
            else if(vPath[nNextPath].start.y > vPath[nNextPath].end.y) return DIR_UP;
            else return DIR_UNDEFINED;
        }
        else{
            return DIR_UNDEFINED;
        }
    }

    bool CalculatePath(std::vector<sPath> vPath, std::vector<olc::vf2d> vNewPath, const int& nOut1, const int& nOut2, const olc::vf2d& vfTarget){
        // out1 sPath correspondiente a vNewPath[0]
        // out2 sPath correspondiente a vNewPath.back
        bool newPathInvert = false;
        std::vector<sPath> vTempPath;

        if(nOut1 == nOut2){
            if(vPath[nOut1].direction == DIR_VERTICAL){
                if((vNewNodes.back().y - vPath[nOut1].start.y) < (vNewNodes[0].y - vPath[nOut1].start.y)) //revisar
                    newPathInvert = true; 
            }
            if(vPath[nOut1].direction == DIR_HORIZONTAL){
                if((vNewNodes.back().x - vPath[nOut1].start.x) < (vNewNodes[0].x - vPath[nOut1].start.x)) //revisar
                    newPathInvert = true; 
            }
        }
        else if(nOut2 < nOut1){
            newPathInvert = true;
        }
        //for newPath
        for(int i=0; i < vPath.size(); i++){
            if(i < nOut1){
                vTempPath.emplace_back(vPath[i]);
            }
            else if(i == nOut1){
                int newPathStart;
                if(!newPathInvert){
                    newPathStart = 0;
                }
                else{
                    newPathStart = vNewPath.size()-1;
                }

                vTempPath.emplace_back(sPath(vPath[i].start, vNewNodes[newPathStart]));

                for(int i = 0, mod = newPathInvert ? 1 : -1; i < vNewPath.size() - 1; i++){
                    vTempPath.emplace_back(sPath(vNewNodes[newPathStart],vNewNodes[newPathStart+mod]));
                    newPathStart += mod;
                }

                if(nOut1 == nOut2){
                    vTempPath.emplace_back(sPath(vNewNodes.back(),vPath[i].end));
                }
            }
            else if(i < nOut2){

            }
            else if(i == nOut2){
                vTempPath.emplace_back(sPath(vNewNodes.back(),vPath[i].end));
            }
            else{
                vPath.emplace_back(vPath[i]);
            }
        }

        // test tempPath
        int intersects = 0;
        for(auto path: vTempPath){
            if(sPath(vfTarget,olc::vf2d(vfTarget.x, ScreenWidth()+1)).DoesIntersect(path.start,path.end)); //overload DoesIntersect(sPath)
                intersects++;
        }

        if(intersects % 2 == 0){ // intersects is even meaning target is outside the path and alt path is required
            vTempPath.clear();
            
            int nOut1Correct;
            int nOut2Correct;
            if(!newPathInvert){
                nOut1Correct = nOut1;
                nOut2Correct = nOut2;
            }
            else{
                nOut1Correct = nOut2;
                nOut2Correct = nOut1;
            }

            for(int i = nOut1Correct; i <= nOut2Correct; i++){
                if(i == nOut1Correct){
                    vTempPath.emplace_back(sPath(vNewPath.back(),vPath[i].end));
                }
                else if(i < nOut2Correct){
                    vTempPath.emplace_back(vPath[i]);
                }
                else if (i == nOut2Correct){
                    int newPathStart;
                    if(!newPathInvert){
                        newPathStart = 0;
                    }
                    else{
                        newPathStart = vNewPath.size()-1;
                    }

                    vTempPath.emplace_back(sPath(vPath[i].start,vNewPath[newPathStart]));

                    for(int i = 0, mod = newPathInvert ? -1 : 1; i < vNewPath.size()-1; i++){
                        vTempPath.emplace_back(sPath(vNewNodes[newPathStart],vNewNodes[newPathStart+mod]));
                        newPathStart += mod;
                    }
                }
                else{

                }
            }
            vPath.clear();
            vPath = vTempPath;
        }
        return false;
    }

    bool OnUserCreate(){
        vPath.emplace_back(sPath(olc::vf2d(nCorner              ,nCorner               ), olc::vf2d(ScreenWidth()-nCorner,nCorner               )));
        vPath.emplace_back(sPath(olc::vf2d(ScreenWidth()-nCorner,nCorner               ), olc::vf2d(ScreenWidth()-nCorner,ScreenHeight()-nCorner)));
        vPath.emplace_back(sPath(olc::vf2d(ScreenWidth()-nCorner,ScreenHeight()-nCorner), olc::vf2d(nCorner              ,ScreenHeight()-nCorner)));
        vPath.emplace_back(sPath(olc::vf2d(nCorner              ,ScreenHeight()-nCorner), olc::vf2d(nCorner              ,nCorner               )));

        vfPos = olc::vf2d(ScreenWidth()/2, ScreenHeight() - nCorner);
        viPosA = vfPos + olc::vi2d(0, -nPosR);
        viPosB = vfPos + olc::vi2d(nPosS, nPosT);
        viPosC = vfPos + olc::vi2d(-nPosS, nPosT);
        return true;
    }

    bool OnUserUpdate(float fElapsedTime){
        Clear(olc::BLANK);
        olc::Pixel pLineColor(olc::WHITE);

        char direction = GetDirection(fElapsedTime);
        if(!bShipFree){
            if(direction == DIR_UP){
                if(vPath[nCurrPath].direction == DIR_VERTICAL){
                    if(vfPos.y > vPath[nCurrPath].end.y && vPath[nCurrPath].start.y > vPath[nCurrPath].end.y){
                        vfPos.y -= fElapsedTime * 75;
                        if (vfPos.y < vPath[nCurrPath].end.y) vfPos.y = vPath[nCurrPath].end.y;
                    }
                    if(vfPos.y > vPath[nCurrPath].start.y && vPath[nCurrPath].end.y > vPath[nCurrPath].start.y){
                        vfPos.y -= fElapsedTime * 75;
                        if (vfPos.y < vPath[nCurrPath].start.y) vfPos.y = vPath[nCurrPath].start.y;
                    }
                }
                else if(vPath[nCurrPath].direction == DIR_HORIZONTAL){
                    if(vfPos == vPath[nCurrPath].end && GetOutDir(vPath, nCurrPath, DIR_END) == DIR_UP){
                        nCurrPath = (nCurrPath == vPath.size()-1 ? 0 : nCurrPath+1);
                    }
                    else if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_UP){
                        nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
                    }
                    else if(GetKey(olc::Key::SPACE).bHeld){
                        vfPosPrev = vfPos;
                        vfPos.y -= fElapsedTime * 75;
                        if(vfPos.y < nCorner) vfPos.y = nCorner;
                        if(vfPos != vfPosPrev){
                            bShipFree = true;
                        }
                    }
                }
            }
            if(direction == DIR_DOWN){
                if(vPath[nCurrPath].direction == DIR_VERTICAL){
                    if(vfPos.y < vPath[nCurrPath].end.y && vPath[nCurrPath].start.y < vPath[nCurrPath].end.y){
                        vfPos.y += fElapsedTime * 75;
                        if (vfPos.y > vPath[nCurrPath].end.y) vfPos.y = vPath[nCurrPath].end.y;
                    }
                    if(vfPos.y < vPath[nCurrPath].start.y && vPath[nCurrPath].end.y < vPath[nCurrPath].start.y){
                        vfPos.y += fElapsedTime * 75;
                        if (vfPos.y > vPath[nCurrPath].start.y) vfPos.y = vPath[nCurrPath].start.y;
                    }
                }
                else if(vPath[nCurrPath].direction == DIR_HORIZONTAL){
                    if(vfPos == vPath[nCurrPath].end && GetOutDir(vPath, nCurrPath, DIR_END) == DIR_DOWN){
                        nCurrPath = (nCurrPath == vPath.size()-1 ? 0 : nCurrPath+1);
                    }
                    else if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_DOWN){
                        nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
                    }
                    else if(GetKey(olc::Key::SPACE).bHeld){
                        vfPosPrev = vfPos;
                        vfPos.y += fElapsedTime * 75;
                        if(vfPos.y > ScreenHeight()-nCorner) vfPos.y = ScreenHeight()-nCorner;
                        if(vfPos != vfPosPrev){
                            bShipFree = true;
                        }
                    }
                }
            }
            if(direction == DIR_LEFT){
                if(vPath[nCurrPath].direction == DIR_HORIZONTAL){
                    if(vfPos.x > vPath[nCurrPath].end.x && vPath[nCurrPath].start.x > vPath[nCurrPath].end.x){
                        vfPos.x -= fElapsedTime * 75;
                        if (vfPos.x < vPath[nCurrPath].end.x) vfPos.x = vPath[nCurrPath].end.x;
                    }
                    if(vfPos.x > vPath[nCurrPath].start.x && vPath[nCurrPath].end.x > vPath[nCurrPath].start.x){
                        vfPos.x -= fElapsedTime * 75;
                        if (vfPos.x < vPath[nCurrPath].start.x) vfPos.x = vPath[nCurrPath].start.x;
                    }
                }
                else if(vPath[nCurrPath].direction == DIR_VERTICAL){
                    if(vfPos == vPath[nCurrPath].end && GetOutDir(vPath, nCurrPath, DIR_END) == DIR_LEFT){
                        nCurrPath = (nCurrPath == vPath.size()-1 ? 0 : nCurrPath+1);
                    }
                    else if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_LEFT){
                        nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
                    }
                    else if(GetKey(olc::Key::SPACE).bHeld){
                        vfPosPrev = vfPos;
                        vfPos.x -= fElapsedTime * 75;
                        if(vfPos.x < nCorner) vfPos.x = nCorner;
                        if(vfPos != vfPosPrev){
                            bShipFree = true;
                        }
                    }
                }
            }
            if(direction == DIR_RIGHT){
                if(vPath[nCurrPath].direction == DIR_HORIZONTAL){
                    if(vfPos.x < vPath[nCurrPath].end.x && vPath[nCurrPath].start.x < vPath[nCurrPath].end.x){
                        vfPos.x += fElapsedTime * 75;
                        if (vfPos.x > vPath[nCurrPath].end.x) vfPos.x = vPath[nCurrPath].end.x;
                    }
                    if(vfPos.x < vPath[nCurrPath].start.x && vPath[nCurrPath].end.x < vPath[nCurrPath].start.x){
                        vfPos.x += fElapsedTime * 75;
                        if (vfPos.x > vPath[nCurrPath].start.x) vfPos.x = vPath[nCurrPath].start.x;
                    }
                }
                else if(vPath[nCurrPath].direction == DIR_VERTICAL){
                    if(vfPos == vPath[nCurrPath].end && GetOutDir(vPath, nCurrPath, DIR_END) == DIR_RIGHT){
                        nCurrPath = (nCurrPath == vPath.size()-1 ? 0 : nCurrPath+1);
                    }
                    else if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_RIGHT){
                        nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
                    }
                    else if(GetKey(olc::Key::SPACE).bHeld){
                        vfPosPrev = vfPos;
                        vfPos.x += fElapsedTime * 75;
                        if(vfPos.x > ScreenWidth()-nCorner) vfPos.x = ScreenWidth()-nCorner;
                        if(vfPos != vfPosPrev){
                            bShipFree = true;
                        }
                    }
                }
            }
        }
        else{
            if(nLastDir == DIR_UNDEFINED){
                vNewNodes.push_back(vfPosPrev);
                nLastDir = direction;
                nPathPrev = nCurrPath;
                nCurrPath = -1;
            }
            else{
                if(direction == DIR_UP){
                    if(nLastDir == DIR_LEFT || nLastDir == DIR_RIGHT){
                        vNewNodes.push_back(vfPosPrev);
                    }
                    vfPos.y -= fElapsedTime * 65;
                    if(vfPos.y < nCorner) vfPos.y = nCorner;
                    nLastDir = DIR_UP;
                    for(int i = 0; i < vPath.size(); i++){
                        if (vPath[i].DoesIntersect(vfPos, vfPosPrev)){
                            nCurrPath = i;
                            vNewNodes.emplace_back(vfPos);
                            CalculatePath(vPath, vNewNodes, nPathPrev, nCurrPath, vfTarget);
                            vNewNodes.clear();
                            bShipFree = false; 
                            nLastDir = DIR_UNDEFINED;
                        }
                    }
                }
                else if(direction == DIR_DOWN){
                    if(nLastDir == DIR_LEFT || nLastDir == DIR_RIGHT){
                        vNewNodes.push_back(vfPosPrev);
                    }
                    vfPos.y += fElapsedTime * 65;
                    if(vfPos.y > ScreenHeight()-nCorner) vfPos.y = ScreenHeight()-nCorner;
                    nLastDir = DIR_DOWN;
                    for(int i = 0; i < vPath.size(); i++){
                        if (vPath[i].DoesIntersect(vfPos, vfPosPrev)){
                            nCurrPath = i;
                            vNewNodes.emplace_back(vfPos);
                            CalculatePath(vPath, vNewNodes, nPathPrev, nCurrPath, vfTarget);
                            vNewNodes.clear();
                            bShipFree = false; 
                            nLastDir = DIR_UNDEFINED;
                        }
                    }
                }
                else if(direction == DIR_LEFT){
                    if(nLastDir == DIR_UP || nLastDir == DIR_DOWN){
                        vNewNodes.push_back(vfPosPrev);
                    }
                    vfPos.x -= fElapsedTime * 65;
                    if(vfPos.x < nCorner) vfPos.x = nCorner;
                    nLastDir = DIR_LEFT;
                    for(int i = 0; i < vPath.size(); i++){
                        if (vPath[i].DoesIntersect(vfPos, vfPosPrev)){
                            nCurrPath = i;
                            vNewNodes.emplace_back(vfPos);
                            CalculatePath(vPath, vNewNodes, nPathPrev, nCurrPath, vfTarget);
                            vNewNodes.clear();
                            bShipFree = false; 
                            nLastDir = DIR_UNDEFINED;
                        }
                    }
                }
                else if(direction == DIR_RIGHT){
                    if(nLastDir == DIR_UP || nLastDir == DIR_DOWN){
                        vNewNodes.push_back(vfPosPrev);
                    }
                    vfPos.x += fElapsedTime * 65;
                    if(vfPos.x > ScreenWidth()-nCorner) vfPos.x = ScreenWidth()-nCorner;
                    nLastDir = DIR_RIGHT;
                    for(int i = 0; i < vPath.size(); i++){
                        if (vPath[i].DoesIntersect(vfPos, vfPosPrev)){
                            nCurrPath = i;
                            vNewNodes.emplace_back(vfPos);
                            CalculatePath(vPath, vNewNodes, nPathPrev, nCurrPath, vfTarget);
                            vNewNodes.clear();
                            bShipFree = false; 
                            nLastDir = DIR_UNDEFINED;
                        }
                    }
                }
                vfPosPrev = vfPos;
            }

        }

        for(int i = 0; i < vPath.size(); i++){
            if(i == nCurrPath) pLineColor = olc::Pixel(0,192,255);
            else pLineColor = olc::WHITE;
            DrawLine(vPath[i].start, vPath[i].end, pLineColor);
        }

        if(bShipFree){
            if(vNewNodes.size() > 1){
                for(int i=1; i < vNewNodes.size(); i++){
                    DrawLine(vNewNodes[i], vNewNodes[i-1], olc::Pixel(255,64,64));
                }
                DrawLine(vNewNodes.back(),vfPos, olc::Pixel(255,64,0));
            }
            else if(vNewNodes.size() == 1){
                DrawLine(vNewNodes[0], vfPos, olc::Pixel(255,64,0));
            }
            
        }

        DrawTriangle(viPosA,viPosB,viPosC, olc::RED);
        DrawLine(vfPos,vfPos, olc::RED);
        
        return true;
    }
};

int main(){
    GAME game_obj;
    if(game_obj.Construct(SCREEN_WIDTH,SCREEN_HEIGHT,PIXEL_SIZE,PIXEL_SIZE)) game_obj.Start();

    return 0;
}
