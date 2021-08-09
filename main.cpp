#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include <iostream>
#include <random>
#include <cstdlib>
#include <ctime>
#include <math.h>


namespace trap
{
    uint32_t nLehmer = 0;

    uint32_t Lehmer32()
    {
        nLehmer += 0xe120fc15;
        uint64_t tmp;
        tmp = (uint64_t)nLehmer * 0x4a39b70d;
        uint32_t m1 = (tmp >> 32) ^ tmp;
        tmp = (uint64_t)m1 * 0x12fad5c9;
        uint32_t m2 = (tmp >> 32) ^ tmp;
        return m2;
    }

    int rndInt(int min, int max)
    {
        return (Lehmer32() % (max - min)) + min;
    }

    double rndDouble(double min, double max)
    {
        return ((double)Lehmer32() / (double)(0x7FFFFFFF)) * (max - min) + min;
    }

    class cStarSystem
    {
        public:
            cStarSystem(uint32_t x, uint32_t y)
            {
                nLehmer = (x & 0xFFFF) << 16 | (y & 0xFFFF);

                starExist = (rndInt(0, 20) == 1);
                
                if(!starExist) return;
                else
                {
                    starDiameter = rndDouble(1.0f, 5.0f);
                }
            }

            bool starExist = false;
            double starDiameter;
    };

    struct tube
    {
   
        olc::vf2d pos;
        olc::vi2d size;
        float tubeXCoord;

        std::array<tube*, 4> contact;

    };


}

class Game : public olc::PixelGameEngine
{
protected:

struct bird
{
    olc::vf2d pos;
    olc::vi2d size;

    olc::vf2d velocity;


};

private:
std::array<trap::tube, 4> a_tube;
bird b;
olc::vf2d screen_sector = {0, 0};
olc::vf2d vCameraPos = {0,0};
olc::vf2d vGalaxyOffset = {0,0};
olc::vf2d tmpPos;
bool bIsStart = false;
bool bIsJump = false;
bool bGameOver = false;
float fOffsetX;
float fTileOffsetX;
int nSectorX;


//Sprites
std::unique_ptr<olc::Sprite> sprBirdFragment;
std::unique_ptr<olc::Decal> decBirdFragment;

std::unique_ptr<olc::Sprite> sprTubeUpFragment;
std::unique_ptr<olc::Decal> decTubeUpFragment;

std::unique_ptr<olc::Sprite> sprTubeDown1Fragment;
std::unique_ptr<olc::Decal> decTubeDown1Fragment;

std::unique_ptr<olc::Sprite> sprTubeDown2Fragment;
std::unique_ptr<olc::Decal> decTubeDown2Fragment;

std::unique_ptr<olc::Sprite> sprBackground;


public:
    Game() { sAppName = "Crappy bird";}
    virtual ~Game(){}

    bool RayVsRect(const olc::vf2d& ray_origin, const olc::vf2d& ray_direction, const trap::tube* target,
     olc::vf2d &contact_point, olc::vf2d &contact_normal, float &t_hit_near)
     {
         contact_point = {0, 0};
         contact_normal = {0, 0};

         olc::vf2d dir = 1.0f / ray_direction;   

         olc::vf2d t_near = (target->pos - ray_origin) * dir;
         olc::vf2d t_far = (target->pos + target->size - ray_origin) * dir;

         //isNaN
         if(std::isnan(t_near.x) || std::isnan(t_near.y)) return false;
         if(std::isnan(t_far.x) || std::isnan(t_far.y)) return false;

        
         if(t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
         if(t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

         //definition Nx < Fy || Ny < Fx
         if(t_near.x > t_far.y || t_near.y > t_far.x) return false;


        t_hit_near = std::max(t_near.x, t_near.y);
        float t_hit_far = std::min(t_far.x, t_far.y);
         
        if(t_hit_far < 0) return false;

        contact_point = (ray_origin + t_hit_near * ray_direction);

        //check near point as soon as far point has checked

        if(t_near.x > t_near.y)
        {
            if(dir.x < 0)
                contact_normal = {1, 0};
            else
                contact_normal = {-1, 0};
        }
        else if(t_near.y > t_near.x)
        {
            if(dir.y < 0)
                contact_normal = {0, 1};
            else
                contact_normal = {0, -1};
        }

        return true;

     }


    bool DynamicRectVsRect(const bird *dynamic, const float fTimeStep, const trap::tube &static_rect, olc::vf2d &cp,
    olc::vf2d &cn, float &contact_time)
    {
        if(dynamic->velocity.x == 0 && dynamic->velocity.y == 0)
            return false;

        //Expanded Rectangle
        trap::tube expanded;

        expanded.pos = static_rect.pos - dynamic->size / 2;
        expanded.size = static_rect.size + dynamic->size;
        
        if(RayVsRect(dynamic->pos + dynamic->size / 2, dynamic->velocity * fTimeStep, &expanded, cp, cn, contact_time))
            return (contact_time < 1.0f && contact_time >= 0.0f);
        else
            return false;

    }

    bool OnUserCreate() override
    {
       //srand(static_cast<unsigned int>(time(0)));



        a_tube[0] = {{150.0f, 0.0f}, {25, 100}}; //up 1
        a_tube[1] = {{200.0f, ScreenHeight() - 80.0f}, {25, 95}}; //down 1
        a_tube[2] = {{250.0f, ScreenHeight() - 80.0f}, {25, 95}}; //down 2
        a_tube[3] = {{300.0f, 0.0f}, {25, 100}}; //up 2

    
       b = {{10.0f, ScreenHeight() / 2.0f}, {16, 16}};
        

       sprBirdFragment = std::make_unique<olc::Sprite>("./gfx/crappy.png");
       decBirdFragment = std::make_unique<olc::Decal>(sprBirdFragment.get());

       sprTubeUpFragment = std::make_unique<olc::Sprite>("./gfx/tubeup.png");
       decTubeUpFragment = std::make_unique<olc::Decal>(sprTubeUpFragment.get());

       sprTubeDown1Fragment = std::make_unique<olc::Sprite>("./gfx/tubedown1.png");
       decTubeDown1Fragment = std::make_unique<olc::Decal>(sprTubeDown1Fragment.get());

       sprBackground = std::make_unique<olc::Sprite>("./gfx/background.png");

        tmpPos = {10.0f, ScreenHeight() / 2.0f};

        return true;
    }

    bool OnUserUpdate(float dt) override
    {
        
        /*
        std::random_device rd; //device from getting random numbers
        std::mt19937 mt(rd()); // engine

        //distribution
        std::uniform_real_distribution<> dist(0, 10);
        */

        vCameraPos = b.pos;
        nSectorX = ScreenWidth() / 25;
        fOffsetX = vCameraPos.x - (float)nSectorX / 2.0f;
        fTileOffsetX = (fOffsetX - (int)fOffsetX) * 25;

        float fAngle = 5.0f;

        olc::vf2d source_point = (b.pos + b.size / 2.0f);
        olc::vf2d normPos = {source_point.x + b.size.x / 2.0f, source_point.y};
        olc::vf2d mousePos = {(float)GetMouseX(), (float)GetMouseY()};
        olc::vf2d ray_direction = normPos - source_point;
        
        olc::vf2d cp, cn;

        float t = 0;
        auto collision1 = [&](const olc::vf2d& point, const bird *r)
        {
            return (point.x >= r->pos.x - fOffsetX && point.y >= r->pos.y && point.x < r->pos.x + r->size.x - fOffsetX && point.y < r->pos.y + r->size.y);
        };

        auto collision2 = [&](const olc::vf2d& a, const trap::tube& t)
        {
            
            return (a.x < 150.0f + (screen_sector.x * t.pos.x + t.size.x)
            && 150.0f + (screen_sector.x * t.pos.x) < a.x + b.size.x
            && a.y < t.pos.y + t.size.y
            && t.pos.y < a.y + b.size.y);

        };

    
vGalaxyOffset.x += 10.0f * dt;


    Clear(olc::CYAN);

        if(IsFocused())
        {
            if(GetKey(olc::SPACE).bPressed)
            {
                 b.velocity.x = (300.0f * (float)cos(fAngle));
                 b.velocity.y -= 70.0f;
                 

                 bGameOver = false;
            }
            else b.velocity.y += 0.2f;
           
        }

if(bGameOver)
{
    b.pos = {tmpPos.x, tmpPos.y};
    b.velocity = {0,0};
}   

bIsStart = (b.velocity.x <= 0) ? false : true;

if(!bIsStart)
{
    b.velocity.y = 0.0f;
    vGalaxyOffset.x = 0;
}


//Clamp velocity
if(b.velocity.x > 300.0f) b.velocity.x = 300.0f;
if(b.velocity.y < -70.0f) b.velocity.y = -70.0f;

//Reach ScreenHeight
if(b.pos.y >= ScreenHeight()) bGameOver = true;

b.pos += b.velocity * dt;

        /*
        for(int x = 0; x < 100; ++x)
        {
            a_tube[0].pos.x = std::floor(dist(mt));
        }
        */

    //BG_STAR_SYSTEM
    for(screen_sector.x = 0; screen_sector.x < nSectorX * 10; screen_sector.x++)
    {
        for(screen_sector.y = 0; screen_sector.y < nSectorX * 10; screen_sector.y++)
        {
            trap::cStarSystem star(screen_sector.x + (uint32_t)vGalaxyOffset.x, screen_sector.y + (uint32_t)vGalaxyOffset.y);

            if(star.starExist)
                FillCircle(screen_sector.x * 16 + 8, screen_sector.y * 16 + 8, (int)star.starDiameter / 8, olc::WHITE);
        }
    }

        //drawing traps
        //up blocks bar

        for(screen_sector.x = 0; screen_sector.x < nSectorX * 10; screen_sector.x++)
        { 
                
            if(collision2(b.pos, a_tube[0])) //&& overlapping(b.pos, a_tube[0].pos))
            {
                DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[0].pos.x - fOffsetX), a_tube[0].pos.y), decTubeUpFragment.get());
                bGameOver = true;
            }
            else
                DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[0].pos.x - fOffsetX), a_tube[0].pos.y), decTubeUpFragment.get());
        }

        //down blocks bar
        for(screen_sector.x = 0; screen_sector.x < nSectorX * 10; screen_sector.x++)
        {
            
            if(collision2(b.pos, a_tube[1])) //&& overlapping(b.pos, a_tube[0].pos))
            {
                DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[1].pos.x - fOffsetX), a_tube[1].pos.y), decTubeDown1Fragment.get());
                bGameOver = true;
            }
            else
                DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[1].pos.x - fOffsetX), a_tube[1].pos.y), decTubeDown1Fragment.get());
              
        }

        //down second bar
        for(screen_sector.x = 0; screen_sector.x < nSectorX * 10; screen_sector.x++)
        {      
            if(collision2(b.pos, a_tube[2])) //&& overlapping(b.pos, a_tube[0].pos))
            {
                DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[2].pos.x - fOffsetX), a_tube[2].pos.y), decTubeDown1Fragment.get());
                bGameOver = true;
            }
            else
                DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[2].pos.x - fOffsetX), a_tube[2].pos.y), decTubeDown1Fragment.get());     

        }

        // up second bar
        for(screen_sector.x = 0; screen_sector.x < nSectorX * 10; screen_sector.x++)
        {       

            if(collision2(b.pos, a_tube[3])) //&& overlapping(b.pos, a_tube[0].pos))
            {
                 DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[3].pos.x - fOffsetX), a_tube[3].pos.y), decTubeUpFragment.get());
                 bGameOver = true;
            }
            else
                DrawDecal(olc::vf2d(150.0f + (screen_sector.x * a_tube[3].pos.x - fOffsetX), a_tube[3].pos.y), decTubeUpFragment.get());  
        }

    

        SetPixelMode(olc::Pixel::MASK);
            DrawRotatedDecal(olc::vf2d(b.pos.x - fOffsetX, b.pos.y), decBirdFragment.get(), fAngle * b.velocity.y * 0.001f);
        SetPixelMode(olc::Pixel::NORMAL);
    
   

    //norm line 
       //DrawLine((b.pos.x + b.size.x / 2) - fOffsetX, b.pos.y + b.size.y / 2, (b.pos.x + b.size.x / 2 + b.velocity.norm().x * 10.0f) - fOffsetX,
        //b.pos.y + b.size.y / 2 + b.velocity.norm().y * 10.0f, olc::RED);
        
       // DrawRect(tmpPos.x - fOffsetX, tmpPos.y, 16, 16, olc::BLACK);

        return true;
    }

};

int main()
{
    Game g;
    if(g.Construct(256, 240, 4, 4, false))
        g.Start();

    return 0;
}
