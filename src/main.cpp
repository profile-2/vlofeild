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

    std::vector<sPath> vPath;
    int nCurrPath = 2;

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
                if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_UP){
                    nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
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
                if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_DOWN){
                    nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
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
                if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_LEFT){
                    nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
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
                if(vfPos == vPath[nCurrPath].start && GetOutDir(vPath, nCurrPath, DIR_START) == DIR_RIGHT){
                    nCurrPath = (nCurrPath == 0 ? vPath.size()-1 : nCurrPath-1);
                }
            }
        }

        for(int i = 0; i < vPath.size(); i++){
            if(i == nCurrPath) pLineColor = olc::Pixel(0,192,255);
            else pLineColor = olc::WHITE;
            DrawLine(vPath[i].start, vPath[i].end, pLineColor);
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
