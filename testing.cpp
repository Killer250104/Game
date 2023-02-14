#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Main.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>

#include<iostream>
#include<vector>
#include<algorithm>
#include<random>
#include<ctime>
#include<cmath>

#define ll int
#define ld float
#define fs first
#define sc second
#define debug(y) cout<<y,exit(0)
using namespace std;
typedef pair<int,int> LL;
const ll N = 2e5 + 9;
const ll mod = 1e9 + 7;
const ld pi = 3.14159265;

mt19937_64 mt(time(0));
ll Rand(ll l,ll r){
    return l + mt() % (r - l + 1);
}

/// standard object size : 80x80 nuke : 80x40

sf::RenderWindow window(sf::VideoMode(1300, 700), "Gaming");
ld v0 = 1;

bool isOnLand(ld x,ld y){
    if (y < 420 || (x > 400 && x < 820) || y > 425) return 0;
    return 1;
}

sf::Texture texture_ground,texture_lf,texture_rg,texture_box, texture_cloud[2], texture_background,
texture_nuke, texture_stuka_lf,texture_stuka_rg,texture_explo_nuke,texture_explo_tnt,texture_tnt,
texture_air_icon,texture_bullet_lf,texture_bullet_rg,texture_bullet_icon, texture_drug,
texture_stun, texture_number[10], texture_sadSS_lf, texture_sadSS_rg, texture_happySS_lf, texture_happySS_rg,
texture_p1_win,texture_p2_win, texture_buff, texture_debuff;


sf::SoundBuffer jump_buffer,explo_nuke_buffer,explo_tnt_buffer,stuka_buffer,bullet_buffer,power_buffer
,outro_buffer;
sf::Sound jumpSound,explo_nukeSound,explo_tntSound,stukaSound,bulletSound,powerSound,outro;
sf::Music main_theme;
const ld speed = 4;
struct object{
    sf::Sprite sprite;
    bool isDraw = 1,is_jump = 0,is_fall = 0,is_picked = 0,is_throw = 0,character = 0,jumping = 0, dizzy = 0;;
    ld t_jump = 0,t_fall = 0,t = 0,direct = 0, odirect = 0,onDrug = 0, speed = 0.03;
    object *carry;

    void renew(){
        is_jump = is_fall = is_picked = is_throw = t_jump = t_fall = t = direct = dizzy = 0;
        carry = NULL;
    }

    object (){
        carry = NULL;
    }

    void setPosition(ld x,ld y){
        sprite.setPosition(x,y);
    }

    void drawObject(){
        if (isDraw) window.draw(sprite);
    }

    void moves(ld x,ld y){
        ld width = sprite.getTexture()->getSize().x;
        sf::Vector2f pos = sprite.getPosition();
        pos.x += x; pos.y += y;
        if(character)
        {
            if(x > 0)
            {
                if(odirect == 0)
                    sprite.setTexture(texture_lf);
                odirect = 1;
            }
            else if(x < 0)
            {
                if(odirect == 1)
                    sprite.setTexture(texture_rg);
                odirect = 0;
            }
            if ((pos.x >= 0 && pos.x <= 390) || (pos.x >= 830 && pos.x <= 1220))
                sprite.move(x,y);
        }
        else
        {
            if (width != 180){
                sprite.move(x, y);
            }
            else{
                if((x > 0 && pos.x > 1220) || (pos.x < 0 && x < 0) ) x *= -1;
                sprite.move(x, y);
            }

        }

    }

    void jump(){

        t_jump += 0.003; //debug(t_jump);
        sf::Vector2f pos = sprite.getPosition();
        if (isOnLand(pos.x,pos.y)) jumpSound.play();
        //cout<<pos.x<<" "<<pos.y<<"\n";
        pos.y = pos.y - v0*t_jump + 0.5*0.98*t_jump*t_jump;

        if (isOnLand(pos.x,pos.y)) is_jump = 0,sprite.setPosition(pos.x,420),t_jump = 0;
        else sprite.setPosition(pos.x,pos.y);
        //cout<<pos.x<<" "<<pos.y<<"\n"; exit(0);
    }

    void fall(ld timing){
        if (is_picked == 1) return;
        t_fall += timing;
        sf::Vector2f pos = sprite.getPosition();
        pos.y = pos.y + 0.5*0.98*t_fall*t_fall;
        if (isOnLand(pos.x,pos.y)) is_jump = 0,sprite.setPosition(pos.x,420),t_fall = 0;
        else sprite.setPosition(pos.x,pos.y);
    }

    void pickup(object &picked){
        carry = &picked;
        picked.is_picked = 1;
    }
    void drop(){
        if (!carry) return;
        (*carry).is_picked = 0; carry = NULL;
    }
    void update_Inventory(){
        sf::Vector2f pos = sprite.getPosition();
        if (!carry) return;
        (*carry).setPosition(pos.x,pos.y - 80);
    }

    void throwStuff(ld dir){
        if (!carry) return;
        (*carry).is_throw = 1;
        (*carry).direct = dir;
        carry = NULL;
    }

    void throwing(ll direct){
        if (!is_throw) return;
        t += 0.003;
        sf::Vector2f pos = sprite.getPosition();
        //cout<<pos.x<<" "<<pos.y<<" x ";
        pos.x += direct*0.95*cos(40.0*pi/180.0)*t;
        pos.y += -0.95*sin(40.0*pi/180.0)*t + 0.5*0.98*t*t;
        is_picked = 0;
        //sprite.setPosition(pos.x,pos.y);
        if (isOnLand(pos.x,pos.y)) renew(),sprite.setPosition(pos.x,420);
        else sprite.setPosition(pos.x,pos.y);
    }
};

void call_stuka(object &stuka,ld x,ld y){
    stuka.isDraw = 1;
    stuka.sprite.setPosition(x,y);
    stukaSound.play();
}

bool CanPick(object &a,object &b){
    if (a.carry) return 0;
    sf::Vector2f pos1 = a.sprite.getPosition();
    sf::Vector2f pos2 = b.sprite.getPosition();
    return (abs(pos1.x - pos2.x) <= 20 && abs(pos1.y - pos2.y) <= 10);
}
int n = 10, t = 0, m = 8200;
object boxes[11], numberx, numbery, cloud[2];
object ground_lf,ground_rg,player_lf,player_rg, background,nuke,stuka_lf,stuka_rg,
explo_nuke,tnt,explo_tnt,air_icon,bullet_lf,bullet_rg,bullet_icon, drug, winner, stun,
happySS_lf,happySS_rg,sadSS_lf,sadSS_rg,p1_win,p2_win, buff, debuff;

sf::IntRect nukeRect(0, 0, 170, 160);
sf::IntRect tntRect(0, 0, 100, 100);
sf::Font font;
sf::Text text;
void Loading(){

    for(int i = 0, x = 25, y = 18; i < 10; i ++)
    {
        texture_number[i].loadFromFile("numbers.png", sf::IntRect(x, y, 72, 72));
        if(i == 3 || i == 7)
        {
            x = 25;
            y += 76;
        }
        else x += 75;
    }
    main_theme.openFromFile("main_theme.wav");

    outro_buffer.loadFromFile("outro.wav");
    outro.setBuffer(outro_buffer);

    texture_p1_win.loadFromFile("player_1_win.png");
    texture_p2_win.loadFromFile("player_2_win.png");
    p1_win.sprite.setTexture(texture_p1_win);
    p2_win.sprite.setTexture(texture_p2_win);

    texture_buff.loadFromFile("buff.png");
    buff.sprite.setTexture(texture_buff);
    texture_debuff.loadFromFile("debuff.png");
    debuff.sprite.setTexture(texture_debuff);
    texture_happySS_lf.loadFromFile("happySS_lf.png");
    texture_happySS_rg.loadFromFile("happySS_rg.png");
    texture_sadSS_lf.loadFromFile("sadSS_lf.png");
    texture_sadSS_rg.loadFromFile("sadSS_rg.png");
    sadSS_lf.sprite.setTexture(texture_sadSS_lf);
    sadSS_rg.sprite.setTexture(texture_sadSS_rg);
    happySS_lf.sprite.setTexture(texture_happySS_lf);
    happySS_rg.sprite.setTexture(texture_happySS_rg);


    texture_stun.loadFromFile("stun.png");
    texture_bullet_icon.loadFromFile("bullet_icon.png");
    texture_bullet_rg.loadFromFile("bullet_rg.png");
    texture_bullet_lf.loadFromFile("bullet_lf.png");
    texture_air_icon.loadFromFile("air_icon.png");
    texture_tnt.loadFromFile("tnt.png");
    texture_explo_tnt.loadFromFile("explo_tnt.png");
    texture_explo_nuke.loadFromFile("explo_nuke.png");
    texture_stuka_lf.loadFromFile("stuka_lf.png");
    texture_stuka_rg.loadFromFile("stuka_rg.png");
    texture_drug.loadFromFile("drug.png", sf::IntRect(20, 0, 40, 50));
    texture_nuke.loadFromFile("nuke.png");
    texture_box.loadFromFile("box.png");
    texture_ground.loadFromFile("ground.png");
    texture_rg.loadFromFile("player_rg.png");
    texture_lf.loadFromFile("player_lf.png");
    texture_cloud[0].loadFromFile("clouds.png", sf::IntRect(60, 60, 180, 180));
    texture_cloud[1].loadFromFile("clouds.png", sf::IntRect(60, 60, 180, 180));
    texture_background.loadFromFile("background.png");
    ground_lf.sprite.setTexture(texture_ground); ground_rg.sprite.setTexture(texture_ground);
    player_lf.sprite.setTexture(texture_lf);
    player_rg.sprite.setTexture(texture_rg);
    //box.sprite.setTexture(texture_box);
    cloud[0].sprite.setTexture(texture_cloud[0]);
    cloud[1].sprite.setTexture(texture_cloud[1]);
    background.sprite.setTexture(texture_background);
    stuka_lf.sprite.setTexture(texture_stuka_lf);
    stuka_rg.sprite.setTexture(texture_stuka_rg);
    for(int i = 0; i <= n; i ++)boxes[i].sprite.setTexture(texture_box);
    nuke.sprite.setTexture(texture_nuke);
    explo_nuke.sprite.setTexture(texture_explo_nuke);
    explo_nuke.sprite.setTextureRect(nukeRect);
    drug.sprite.setTexture(texture_drug);
    tnt.sprite.setTexture(texture_tnt);
    explo_tnt.sprite.setTexture(texture_explo_tnt);
    explo_tnt.sprite.setTextureRect(tntRect);

    air_icon.sprite.setTexture(texture_air_icon);

    bullet_lf.sprite.setTexture(texture_bullet_lf);
    bullet_rg.sprite.setTexture(texture_bullet_rg);
    bullet_icon.sprite.setTexture(texture_bullet_icon);

    jump_buffer.loadFromFile("jump.wav");
    jumpSound.setBuffer(jump_buffer);

    explo_nuke_buffer.loadFromFile("explo_nuke.wav");
    explo_nukeSound.setBuffer(explo_nuke_buffer);

    explo_tnt_buffer.loadFromFile("explo_tnt.wav");
    explo_tntSound.setBuffer(explo_tnt_buffer);

    stuka_buffer.loadFromFile("stuka_sound.wav");
    stukaSound.setBuffer(stuka_buffer);

    bullet_buffer.loadFromFile("bullet_sound.wav");
    bulletSound.setBuffer(bullet_buffer);

    stun.sprite.setTexture(texture_stun);

    power_buffer.loadFromFile("power_up.wav");
    powerSound.setBuffer(power_buffer);
}

bool is_falling(object box){
    if (!box.isDraw) return 0;
    sf::Vector2f pos = box.sprite.getPosition();
    return (!isOnLand(pos.x,pos.y) && !box.is_throw);
}



void respawn(){

    for(int i = 0; i <= min(n, t/m); i ++)
    {
        sf::Vector2f pos = boxes[i].sprite.getPosition();
        if (pos.y > 700 || pos.x < -80 || pos.x > 1300){
            if (boxes[i].direct == 1) boxes[i].setPosition(Rand(0,320),0);
            else boxes[i].setPosition(Rand(900,1220),0);
            boxes[i].renew();
        }
    }
    if (!tnt.isDraw){
        ll p = Rand(0,1); tnt.isDraw = 1;
        if (p) tnt.setPosition(Rand(0,320),0);
        else tnt.setPosition(Rand(900,1220),0);
    }
}

bool checkplayerdurg(object& player, object& drug){
    sf::Vector2f pos1 = player.sprite.getPosition();
    sf::Vector2f pos2 = drug.sprite.getPosition();
    bool bx = (pos1.x < pos2.x ? pos1.x+80 >= pos2.x : pos2.x+40 >= pos1.x);
    bool by = (pos1.y < pos2.y ? pos1.y+80 >= pos2.y : pos2.y+50 >= pos1.y);
    return bx && by;
}

bool checkplayertnt(object& player, ld x, ld y){
    sf::Vector2f pos = player.sprite.getPosition();
    return abs(pos.x-x) <= 60 && abs(pos.y-y) <= 60;
}

void checkColision(){
    //sf::Vector2f pos1 = box.sprite.getPosition();
    sf::Vector2f pos2 = air_icon.sprite.getPosition(),pos1;
    //cout<<abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y); exit(0);
    //cout<<air_icon.isDraw<<"\n";

    for(int i = 0; i <= min(n, t/m); i ++)
    {
        pos1 = boxes[i].sprite.getPosition();
        //cout<<abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y); exit(0);
        //cout<<air_icon.isDraw<<"\n";
        if (air_icon.isDraw == 1 && abs(pos1.x - pos2.x) <= 80 && abs(pos1.y - pos2.y) <= 80){
            air_icon.isDraw = 0;
            if (boxes[i].direct == 1)
                call_stuka(stuka_lf,-200,0);
            else if (boxes[i].direct == -1)
                call_stuka(stuka_rg,1400,0);
        }
    }
}

sf::Clock timerCallAir, timerdrug, timerspeedrise, timerspeedslow;
sf::Clock timerNuke,timerTnt, timerdizzy_lf, timerdizzy_rg, timer, timerbullet;
bool ended = 0;

bool is_eaten(object &a,object &b){
    sf::Vector2f pos1 = a.sprite.getPosition();
    sf::Vector2f pos2 = b.sprite.getPosition();
    return (abs(pos1.x - pos2.x) <= 20 && pos1.y == pos2.y);
}

void cloud_move(object& cloud)
{
    sf::Vector2f pos = cloud.sprite.getPosition();
    pos.x += cloud.speed;
    if((cloud.speed < 0 && pos.x < 0) || (cloud.speed > 0 && pos.x > 1120))cloud.speed *= -1;
    cloud.sprite.move(cloud.speed, 0);
}

void setspeed(object &o)
{
    if(o.speed < speed)
    {
        debuff.isDraw = 1;
        debuff.isDraw = 1;
        sf::Vector2f pos = o.sprite.getPosition();
        debuff.setPosition(pos.x+10, pos.y-80);
    }
    else if(o.speed > speed)
    {
        buff.isDraw = 1;
        sf::Vector2f pos = o.sprite.getPosition();
        buff.setPosition(pos.x+10, pos.y-60);
    }
}

void Change_Gamestate(){
    buff.isDraw = 0;
    debuff.isDraw = 0;
    //if (box.is_throw) box.throwing(box.direct);
    for(int i = 0; i <= min(n, t/m); i ++){
        if(boxes[i].is_throw)
            boxes[i].throwing(boxes[i].direct);
    }
    if (tnt.is_throw){
        tnt.throwing(tnt.direct);
        sf::Vector2f pos = tnt.sprite.getPosition();
        //cout << pos.x <<" "<<pos.y<<endl;
        if (isOnLand(pos.x,pos.y) ){

            tnt.isDraw = 0;
            tnt.is_throw = 0;
            explo_tnt.isDraw = 1; explo_tnt.setPosition(pos.x,pos.y);
            explo_tntSound.play();
            if(checkplayertnt(player_lf, pos.x, pos.y))
            {
                player_lf.speed /= 2;
                timerspeedslow.restart();
            }
            if(checkplayertnt(player_rg, pos.x, pos.y))
            {
                player_rg.speed /= 2;
                timerspeedslow.restart();
            }
        }
        else if(pos.y > 1400)
        {
            tnt.isDraw = 0;
            tnt.is_throw = 0;
        }
    }

    //cout<<timerCallAir.getElapsedTime().asSeconds()<<"\n";
    //cout<<box.direct<<"\n";
    //if (box.direct != 0) checkColision();
    for(int i = 0; i <= min(n, t/m); i ++)
        if (boxes[i].direct != 0) checkColision();

    if (air_icon.isDraw == 0){
        if(timerCallAir.getElapsedTime().asSeconds() > 12.f){
            air_icon.isDraw = 1;
            ld x = Rand(370,850),y = Rand(200,460);
            air_icon.setPosition(x,y);
            timerCallAir.restart();
        }
    }
    else timerCallAir.restart();

    if(!drug.isDraw)
    {
        if(timerdrug.getElapsedTime().asSeconds() > 12)
        {
            drug.isDraw = 1;
            timerdrug.restart();
            if(Rand(1, 10)&1)drug.setPosition(Rand(0, 340), Rand(300, 400));
            else drug.setPosition(Rand(900, 1240), Rand(300, 400));
        }

    }
    else
    {
        if(checkplayerdurg(player_lf, drug))
        {
            player_lf.speed *= 2;
            drug.isDraw = 0;
            timerdrug.restart();
            timerspeedrise.restart();
            if (!player_lf.onDrug){
                player_lf.onDrug = 1; powerSound.play();
            }
        }
        else if(checkplayerdurg(player_rg, drug))
        {
            player_rg.speed *= 2;
            drug.isDraw = 0;
            timerdrug.restart();
            timerspeedrise.restart();
            if (!player_rg.onDrug){
                player_rg.onDrug = 1; powerSound.play();
            }
        }
        else if(timerdrug.getElapsedTime().asSeconds() > 10)
        {
            drug.isDraw = 0;
            timerdrug.restart();

        }
    }
    if (player_lf.is_jump){
        player_lf.jump(); //jumpSound.play();
    }
    player_lf.update_Inventory();

    if (player_rg.is_jump){
        player_rg.jump(); //jumpSound.play();
    }
    player_rg.update_Inventory();

    //if (is_falling(box)) box.fall(0.0007);
    for(int i = 0; i <= min(n, t/m); i ++)boxes[i].fall(0.0007);
    if (is_falling(nuke)) nuke.fall(0.003);
    if (is_falling(tnt)) tnt.fall(0.0007);
    cloud_move(cloud[0]);
    cloud_move(cloud[1]);
    if (nuke.isDraw){
        sf::Vector2f pos = nuke.sprite.getPosition();
        if (isOnLand(pos.x,pos.y)){
            nuke.isDraw = 0;
            explo_nuke.isDraw = 1; explo_nuke.setPosition(pos.x - 50,360);
            explo_nukeSound.play();
            if(checkplayertnt(player_lf, pos.x-50, 360))
            {
                player_lf.dizzy = 1;
                sf::Vector2f pos1 = player_lf.sprite.getPosition();
                stun.setPosition(pos1.x,380); stun.isDraw = 1;
                //timerdizzy_lf.restart();
            }
            else if(checkplayertnt(player_rg, pos.x-50, 360))
            {
                player_rg.dizzy = 1;
                sf::Vector2f pos1 = player_rg.sprite.getPosition();
                stun.setPosition(pos1.x,380); stun.isDraw = 1;
                //timerdizzy_rg.restart();
            }
        }
    }

    if (explo_nuke.isDraw){

        if(timerNuke.getElapsedTime().asSeconds() > 0.07f){
            if (nukeRect.left == 680){
                if (nukeRect.top == 640)
                    explo_nuke.isDraw = 0,nukeRect = sf::IntRect(0,0,170,160);
                else nukeRect.left = 0,nukeRect.top += 160;
            }
            else nukeRect.left += 170;
            explo_nuke.sprite.setTextureRect(nukeRect);
            timerNuke.restart();
        }
        //explo_nuke.isDraw = 1;

    }

    if (explo_tnt.isDraw){
        if(timerTnt.getElapsedTime().asSeconds() > 0.1f){
            if (tntRect.left == 600){
                if (tntRect.top == 0)
                    explo_tnt.isDraw = 0,tntRect = sf::IntRect(0,0,100,100);
                //else nukeRect.left = 0,nukeRect.top += 100;
            }
            else tntRect.left += 100;
            explo_tnt.sprite.setTextureRect(tntRect);
            timerTnt.restart();
        }
    }

    if (stuka_lf.isDraw){
        stuka_lf.moves(0.5,0);

        sf::Vector2f pos = stuka_lf.sprite.getPosition();
        if (pos.x > 1200) stuka_lf.isDraw = 0;

        sf::Vector2f enemy = player_rg.sprite.getPosition();

        if (abs(pos.x + 80 - enemy.x) <= 10){
            nuke.isDraw = 1;
            nuke.setPosition(pos.x + 80,pos.y);
        }
    }

     if (stuka_rg.isDraw){
        stuka_rg.moves(-0.5,0);

        sf::Vector2f pos = stuka_rg.sprite.getPosition();
        if (pos.x <= -100) stuka_rg.isDraw = 0;

        sf::Vector2f enemy = player_lf.sprite.getPosition();

        if (abs(pos.x + 80 - enemy.x) <= 1){
            nuke.isDraw = 1;
            nuke.setPosition(pos.x + 80,pos.y);
        }
    }


    if (is_falling(bullet_icon)) bullet_icon.fall(0.0002);
    if (bullet_icon.isDraw){
        sf::Vector2f pos = bullet_icon.sprite.getPosition();
        if (isOnLand(pos.x,pos.y)){
            if (is_eaten(player_rg,bullet_icon)){
                bullet_icon.isDraw = 0;
                bullet_rg.isDraw = 1; bullet_rg.setPosition(1400 - 98,430); bulletSound.play();
            }
            else if (is_eaten(player_lf,bullet_icon)){
                bullet_icon.isDraw = 0;
                bullet_lf.isDraw = 1; bullet_lf.setPosition(-100,430); bulletSound.play();
            }
        }
    }
    else if (!bullet_icon.isDraw && timerbullet.getElapsedTime().asSeconds() > 15.f){
        ll p = Rand(0,1); //p = 0;
        ll x,y = 0;
        if (p) x = Rand(0,400 - 70);
        else x = Rand(900,1300 - 70);
        bullet_icon.setPosition(x,y); bullet_icon.isDraw = 1;
        timerbullet.restart();
    }
    if (bullet_lf.isDraw){
        bullet_lf.moves(1.3,0);
        sf::Vector2f pos = bullet_lf.sprite.getPosition();
        if (pos.x > 1400) bullet_lf.isDraw = 0;

        if(checkplayertnt(player_rg, pos.x, pos.y))
        {
            player_rg.speed /= 2;
            timerspeedslow.restart();
            bullet_lf.isDraw = 0;
        }
    }
    if (bullet_rg.isDraw){
        bullet_rg.moves(-1.3,0);
        sf::Vector2f pos = bullet_rg.sprite.getPosition();
        if (pos.x < -100) bullet_rg.isDraw = 0;
        if(checkplayertnt(player_lf, pos.x, pos.y))
        {
            player_lf.speed /= 2;
            timerspeedslow.restart();
            bullet_rg.isDraw = 0;
        }

    }

    if(timerspeedrise.getElapsedTime().asSeconds() > 12)
    {
        if(player_lf.speed > speed)player_lf.speed = speed;
        if(player_rg.speed > speed)player_rg.speed = speed;
        player_lf.onDrug = player_rg.onDrug = 0;
    }
    if(timerspeedslow.getElapsedTime().asSeconds() > 12)
    {
        if(player_lf.speed < speed)player_lf.speed = speed;
        if(player_rg.speed < speed)player_rg.speed = speed;
    }

    setspeed(player_lf);
    setspeed(player_rg);
    respawn();
}

void RenderGame(){

    background.drawObject();
    cloud[0].drawObject();
    cloud[1].drawObject();
    ground_lf.drawObject(); ground_rg.drawObject();
    for(int i = 0; i <= min(n, t/m); i ++)
    {
        boxes[i].isDraw = 1;
        boxes[i].drawObject();
    }
    //box.drawObject();
    tnt.drawObject();
    player_lf.drawObject();  player_rg.drawObject();
    nuke.drawObject();
    explo_nuke.drawObject();


    explo_tnt.drawObject();

    stuka_lf.drawObject();
    stuka_rg.drawObject();

    bullet_lf.drawObject(); bullet_rg.drawObject(); bullet_icon.drawObject();
    air_icon.drawObject();
    drug.drawObject();

    if(t < n*m)t += 1;
    int now = 100-timer.getElapsedTime().asSeconds();
    if(now <= 0)ended = 1;
    numberx.sprite.setTexture(texture_number[now/10]);
    numbery.sprite.setTexture(texture_number[now%10]);

    numberx.setPosition(580, 50);
    numbery.setPosition(650, 50);
    numberx.drawObject();
    numbery.drawObject();
    stun.drawObject();
    buff.drawObject();
    debuff.drawObject();
}

void init_position(){
    player_rg.odirect = 1;
    player_rg.character = 1;
    player_lf.character = 1;
    background.setPosition(0, 0);
    ground_lf.setPosition(0,500); ground_rg.setPosition(900,500);
    player_lf.setPosition(0,420); player_rg.setPosition(1220,420);
    winner.setPosition(0, 0);
    player_lf.speed = speed;
    player_rg.speed = speed;
    //box.setPosition(Rand(900,1220),0);
    for(int i = 0; i <= n; i ++)
    {
        if(i&1)boxes[i].setPosition(Rand(0,320),0);
        else
            boxes[i].setPosition(Rand(900,1220),0);
        boxes[i].isDraw = 0;
    }

    cloud[0].setPosition(100, 100);
    cloud[1].setPosition(500, 160);

    air_icon.setPosition(Rand(370,850),Rand(200,460));

    stuka_lf.isDraw = stuka_rg.isDraw = nuke.isDraw = explo_nuke.isDraw = drug.isDraw = 0;
    explo_tnt.isDraw = 0; tnt.isDraw = 1;
    //bullet_lf.setPosition(0,430);
    bullet_lf.isDraw = bullet_rg.isDraw = bullet_icon.isDraw = 0;
    stun.isDraw = buff.isDraw = debuff.isDraw =0;
    //bullet_icon.isDraw = 0;

}

bool outro_playing = 0;

void fend()
{
    main_theme.stop();
    window.clear(sf::Color::Black);
    sf::Vector2f pos;
    background.drawObject();
    int cnt = 0;
    for(int i = 0; i <= min(n, t/m); i ++)
    {
        pos = boxes[i].sprite.getPosition();
        cnt += (pos.x <= 400.f);
    }
    if (!outro_playing) outro.play(),outro_playing = 1;
    font.loadFromFile("arial.ttf");
    if(cnt < 6)
    {
        p1_win.setPosition(260,80); p1_win.drawObject();
        happySS_lf.setPosition(0,320); sadSS_rg.setPosition(922,320);
        happySS_lf.drawObject(); sadSS_rg.drawObject();
        window.draw(text);
    }
    else
    {
        p2_win.setPosition(260,80); p2_win.drawObject();
        happySS_rg.setPosition(720,320); sadSS_lf.setPosition(0,320);
        happySS_rg.drawObject(); sadSS_lf.drawObject();
        window.draw(text);
    }
    window.display();
}

/*

Note
left_player: AD to move, W to jump, S to pick up, F to drop, space to throw
right_player: left right up down, K L to throw and pick up

*/
bool started = 1;
ll enter_pressed = 0;
void fpause(){
    font.loadFromFile("arial.ttf");
    window.clear(sf::Color::White);
    background.drawObject();
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(1000, 600));
    rect.setFillColor(sf::Color::Black);
    rect.setPosition(150, 50);
    if (enter_pressed){
        sf::Text text1("Player 1: \nA: move left \nD: move right \nW: to jump \nS: to pick up items \nSpace: to throw items \nF: to drop items", font);
        sf::Text text2("Player 2: \nLeft: move left \nRight: move right \nUp: to jump \nDown: to pick up items \nL: to throw items \nK: to drop items", font);
        sf::Text text3("Press Enter to play now", font);
        text1.setCharacterSize(30);
        text1.setStyle(sf::Text::Bold);
        text1.setFillColor(sf::Color::Yellow);
        text1.setPosition(300, 150);
        text2.setCharacterSize(30);
        text2.setStyle(sf::Text::Bold);
        text2.setFillColor(sf::Color::Yellow);
        text2.setPosition(700, 150);
        text3.setCharacterSize(40);
        text3.setStyle(sf::Text::Bold);
        text3.setFillColor(sf::Color::Red);
        text3.setPosition(400, 500);
        window.draw(rect);
        window.draw(text1);
        window.draw(text2);
        window.draw(text3);
        window.display();
    }
    else{
        sf::Text text1("Here come a short instruction about our world, it's 2077\nwhen nothing happen in tiananmen square but suddenly\nmany box start to fall from sky and hoomins always hate box!!!\nSo they try to throw box to each other so who get more box\nbecome loser. Cuz it's 2077 so even our dudes have tnt in their\nhouse but tnt this time only slow human down thanks to their\nadvance in AI but they also can call air support to send nuke!!!\nBut again nuke only make 2077 hoomins stun for 5 sec\nWe have some breakthrough in medical too, thanks to Dr.bakha\nnow we can take the red pill and become fast for some seconds", font);
        //sf::Text text2("Player 2: \nLeft: move left \nRight: move right \nUp: to jump \nDown: to pick up items \nL: to throw items \nK: to drop items", font);
        sf::Text text3("Press Enter to continue", font);
        text1.setCharacterSize(30);
        text1.setStyle(sf::Text::Bold);
        text1.setFillColor(sf::Color::Yellow);
        text1.setPosition(200, 100);
        text3.setCharacterSize(40);
        text3.setStyle(sf::Text::Bold);
        text3.setFillColor(sf::Color::Red);
        text3.setPosition(400, 500);
        window.draw(rect);
        window.draw(text1);
        window.draw(text3);
        window.display();
    }

}
int main(){
    Loading();
    init_position(); main_theme.play();
    while (window.isOpen()){
        sf::Event event;
        if(started)
        {
            while (true)
            {
                window.pollEvent(event);
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                    return 0;
                }
                if (event.type == sf::Event::KeyPressed){
                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        window.close();
                        return 0;
                    }
                    if(event.key.code == sf::Keyboard::Enter)
                    {
                        enter_pressed++;
                        if (enter_pressed == 2) started = 0;
                        timerCallAir.restart();
                        timerdrug.restart();
                        timerspeedrise.restart();
                        timerspeedslow.restart();
                        timerNuke.restart();
                        timerTnt.restart();
                        timerdizzy_lf.restart();
                        timerdizzy_rg.restart();
                        timer.restart();
                        timerbullet.restart();
                        break;
                    }
                }
                fpause();
            }
        }
        if(ended)
        {
            while (true)
            {
                window.pollEvent(event);
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                    return 0;
                }
                if (event.type == sf::Event::KeyPressed){
                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        window.close();
                        return 0;
                    }
                }
                fend();
            }

        }
        else
        {
            while (window.pollEvent(event)){

                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
                if (event.type == sf::Event::KeyPressed){
                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        window.close();
                    }
                }

                ///left player operation
                //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) call_stuka(stuka_lf,-200,0);
                if(player_lf.dizzy)
                {
                    stun.isDraw = 1;
                    if(timerdizzy_lf.getElapsedTime().asSeconds() > 6.0f){
                        player_lf.dizzy = 0;
                        stun.isDraw = 0;
                    }
                }
                else
                {
                    timerdizzy_lf.restart();
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) player_lf.is_jump = 1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) player_lf.moves(-player_lf.speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
                        //if (CanPick(player_lf,box)) player_lf.pickup(box);
                        if (CanPick(player_lf,tnt)) player_lf.pickup(tnt);
                        else
                        {
                            for(int i = 0; i <= min(n, t/m); i ++)
                                if(CanPick(player_lf, boxes[i]))player_lf.pickup(boxes[i]);
                        }
                    }


                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
                        if (player_lf.carry != NULL) player_lf.drop();

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) player_lf.moves(player_lf.speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) player_lf.throwStuff(1);
                }

                ///

                ///right player operation
                //if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
                if(player_rg.dizzy)
                {
                    stun.isDraw = 1;
                    if(timerdizzy_rg.getElapsedTime().asSeconds() > 6.0f){
                        player_rg.dizzy = 0;
                        stun.isDraw = 0;
                    }

                }
                else
                {
                    timerdizzy_rg.restart();
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) player_rg.is_jump = 1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) player_rg.moves(-player_rg.speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
                        //if (CanPick(player_rg,box)) player_rg.pickup(box);
                        if (CanPick(player_rg,tnt)) player_rg.pickup(tnt);
                        else
                        {
                            for(int i = 0; i <= min(n, t/m); i ++)
                            {
                                if(CanPick(player_rg, boxes[i]))
                                    player_rg.pickup(boxes[i]);
                            }
                        }
                    }



                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))
                        if (player_rg.carry != NULL) player_rg.drop();

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) player_rg.moves(player_rg.speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) player_rg.throwStuff(-1);
                }
                ///



            }
            Change_Gamestate();
            window.clear(sf::Color::Black);
            RenderGame();
            //cloud.moves(10, 10);
            window.display();
        }

    }
    return 0;
}
/* be confident */

