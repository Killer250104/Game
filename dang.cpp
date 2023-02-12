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

sf::Texture texture_ground,texture_lf,texture_rg,texture_box, texture_cloud, texture_background,
texture_nuke, texture_stuka_lf,texture_stuka_rg,texture_explo_nuke,texture_explo_tnt,texture_tnt,
texture_air_icon,texture_bullet_lf,texture_bullet_rg,texture_bullet_icon, texture_drug, texture_winner, texture_number[10];

sf::SoundBuffer jump_buffer,explo_nuke_buffer,explo_tnt_buffer,stuka_buffer,bullet_buffer;
sf::Sound jumpSound,explo_nukeSound,explo_tntSound,stukaSound,bulletSound;

ld cloud_speed = 0.03, player_lf_speed = 5,  player_rg_speed = 5;
struct object{
    sf::Sprite sprite;
    bool isDraw = 1,is_jump = 0,is_fall = 0,is_picked = 0,is_throw = 0,character = 0,jumping = 0, dizzy = 0;;
    ld t_jump = 0,t_fall = 0,t = 0,direct = 0, odirect = 0;
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

    void cloud_move()
    {
        sf::Vector2f pos = sprite.getPosition();
        pos.x += cloud_speed;
        if((cloud_speed < 0 && pos.x < 0) || (cloud_speed > 0 && pos.x > 1220))cloud_speed *= -1;
        sprite.move(cloud_speed, 0);
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
int n = 10, t = 0, m = 6200;
object boxes[10], numberx, numbery;
object ground_lf,ground_rg,player_lf,player_rg,box, cloud, background,nuke,stuka_lf,stuka_rg,
explo_nuke,tnt,explo_tnt,air_icon,bullet_lf,bullet_rg,bullet_icon, drug, winner;

sf::IntRect nukeRect(0, 0, 170, 160);
sf::IntRect tntRect(0, 0, 100, 100);
sf::Font font;
sf::Text text;
void Loading(){


    //text.setPosition(400, 200);
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
    texture_winner.loadFromFile("victory.png");
    texture_bullet_icon.loadFromFile("bullet_icon.png");
    texture_bullet_rg.loadFromFile("bullet_rg.png");
    texture_bullet_lf.loadFromFile("bullet_lf.png");
    texture_air_icon.loadFromFile("air_icon.png");
    texture_tnt.loadFromFile("tnt.png");
    texture_explo_tnt.loadFromFile("explo_tnt.png");
    texture_explo_nuke.loadFromFile("explo_nuke.png");
    texture_stuka_lf.loadFromFile("stuka_lf.png");
    texture_stuka_rg.loadFromFile("stuka_rg.png");
    texture_drug.loadFromFile("drug.png");
    texture_nuke.loadFromFile("nuke.png");
    texture_box.loadFromFile("box.png");
    texture_ground.loadFromFile("ground.png");
    texture_rg.loadFromFile("player_rg.png");
    texture_lf.loadFromFile("player_lf.png");
    texture_cloud.loadFromFile("clouds.png", sf::IntRect(60, 60, 180, 180));
    texture_background.loadFromFile("background.png");
    ground_lf.sprite.setTexture(texture_ground); ground_rg.sprite.setTexture(texture_ground);
    player_lf.sprite.setTexture(texture_lf);
    player_rg.sprite.setTexture(texture_rg);
    box.sprite.setTexture(texture_box);
    cloud.sprite.setTexture(texture_cloud);
    background.sprite.setTexture(texture_background);
    stuka_lf.sprite.setTexture(texture_stuka_lf);
    stuka_rg.sprite.setTexture(texture_stuka_rg);
    for(int i = 0; i < n; i ++)boxes[i].sprite.setTexture(texture_box);
    winner.sprite.setTexture(texture_winner);
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
}

bool is_falling(object box){
    if (!box.isDraw) return 0;
    sf::Vector2f pos = box.sprite.getPosition();
    return (!isOnLand(pos.x,pos.y) && !box.is_throw);
}



void respawn(){
    sf::Vector2f pos = box.sprite.getPosition();
    if (pos.y > 700 || pos.x < -80 || pos.x > 1300){
        if (box.direct == 1) box.setPosition(Rand(0,320),0);
        else box.setPosition(Rand(900,1220),0);
        box.renew();
    }
    for(int i = 0; i < min(n, t/m); i ++)
    {
        sf::Vector2f pos = boxes[i].sprite.getPosition();
        if (pos.y > 700 || pos.x < -80 || pos.x > 1300){
            if (boxes[i].direct == 1) boxes[i].setPosition(Rand(0,320),0);
            else boxes[i].setPosition(Rand(900,1220),0);
            boxes[i].renew();
        }
    }
}

bool checkplayerdurg(object& player, object& drug)
{
    sf::Vector2f pos1 = player.sprite.getPosition();
    sf::Vector2f pos2 = drug.sprite.getPosition();
    bool bx = (pos1.x < pos2.x ? pos1.x+80 >= pos2.x : pos2.x+60 >= pos1.x);
    bool by = (pos1.y < pos2.y ? pos1.y+80 >= pos2.y : pos2.y+60 >= pos1.y);
    return bx && by;
}

bool checkplayertnt(object& player, ld x, ld y)
{
    sf::Vector2f pos1 = player.sprite.getPosition();
    return abs(pos1.x-x) <= 90 && abs(pos1.y-y) <= 90;
}

void checkColision(){
    sf::Vector2f pos1 = box.sprite.getPosition();
    sf::Vector2f pos2 = air_icon.sprite.getPosition();
    //cout<<abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y); exit(0);
    //cout<<air_icon.isDraw<<"\n";
    if (air_icon.isDraw == 1 && abs(pos1.x - pos2.x) <= 80 && abs(pos1.y - pos2.y) <= 80){
        air_icon.isDraw = 0;
        if (box.direct == 1)
            call_stuka(stuka_lf,-200,0);
        else if (box.direct == -1)
            call_stuka(stuka_rg,1400,0);
    }
    for(int i = 0; i < min(n, t/m); i ++)
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
    return (abs(pos1.x - pos2.x) <= 10 && pos1.y == pos2.y);
}

void Change_Gamestate(){
    if (box.is_throw) box.throwing(box.direct);
    for(int i = 0; i < min(n, t/m); i ++){
        if(boxes[i].is_throw)
            boxes[i].throwing(boxes[i].direct);
    }
    if (tnt.is_throw){
        tnt.throwing(tnt.direct);
        sf::Vector2f pos = tnt.sprite.getPosition();
        if (isOnLand(pos.x,pos.y)){
            tnt.isDraw = 0;
            explo_tnt.isDraw = 1; explo_tnt.setPosition(pos.x,pos.y);
            explo_tntSound.play();
            if(checkplayertnt(player_lf, pos.x, pos.y))
            {
                player_lf_speed /= 1.7;
                timerspeedslow.restart();
            }
            if(checkplayertnt(player_rg, pos.x, pos.y))
            {
                player_rg_speed /= 1.7;
                timerspeedslow.restart();
            }
        }
    }

    //cout<<timerCallAir.getElapsedTime().asSeconds()<<"\n";
    //cout<<box.direct<<"\n";
    if (box.direct != 0) checkColision();
    for(int i = 0; i < min(n, t/m); i ++)
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
        if(timerdrug.getElapsedTime().asSeconds() > 17.f)
        {
            drug.isDraw = 1;
            timerCallAir.restart();
            if(Rand(1, 10)&1)drug.setPosition(Rand(0, 340), Rand(300, 400));
            else drug.setPosition(Rand(900, 1240), Rand(300, 400));
        }
    }
    else
    {
        if(checkplayerdurg(player_lf, drug))
        {
            player_lf_speed *= 1.7;
            drug.isDraw = 0;
            timerdrug.restart();
            timerspeedrise.restart();
        }
        else if(checkplayerdurg(player_rg, drug))
        {
            player_rg_speed *= 1.7;
            drug.isDraw = 0;
            timerdrug.restart();
            timerspeedrise.restart();
        }
        else if(timerdrug.getElapsedTime().asSeconds() > 24.f)
        {
            drug.isDraw = 0;
            timerdrug.restart();
        }
    }

    if(timerspeedrise.getElapsedTime().asSeconds() > 10.f)
    {
        if(player_lf_speed > 5)player_lf_speed = 5;
        if(player_rg_speed > 5)player_rg_speed = 5;
    }
    if(timerspeedslow.getElapsedTime().asSeconds() > 10.f)
    {
        if(player_lf_speed < 5)player_lf_speed = 5;
        if(player_rg_speed < 5)player_rg_speed = 5;
    }

    if (player_lf.is_jump){
        player_lf.jump(); //jumpSound.play();
    }
    player_lf.update_Inventory();

    if (player_rg.is_jump){
        player_rg.jump(); //jumpSound.play();
    }
    player_rg.update_Inventory();

    if (is_falling(box)) box.fall(0.0007);
    for(int i = 0; i < min(n, t/m); i ++)boxes[i].fall(0.0007);
    if (is_falling(nuke)) nuke.fall(0.003);
    if (is_falling(tnt)) tnt.fall(0.0007);
    cloud.cloud_move();

    if (nuke.isDraw){
        sf::Vector2f pos = nuke.sprite.getPosition();
        if (isOnLand(pos.x,pos.y)){
            nuke.isDraw = 0;
            explo_nuke.isDraw = 1; explo_nuke.setPosition(pos.x - 50,360);
            explo_nukeSound.play();
            if(checkplayertnt(player_lf, pos.x-50, 360))
            {
                player_lf.dizzy = 1;
                timerdizzy_lf.restart();
            }
            if(checkplayertnt(player_rg, pos.x-50, 360))
            {
                player_rg.dizzy = 1;
                timerdizzy_rg.restart();
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
                bullet_lf.isDraw = 1; bullet_lf.setPosition(-100,430); bulletSound.play();
            }
            else if (is_eaten(player_lf,bullet_icon)){
                bullet_icon.isDraw = 0;
                bullet_rg.isDraw = 1; bullet_rg.setPosition(1400 - 98,430); bulletSound.play();
            }
        }
    }
    else if (!bullet_icon.isDraw && timerbullet.getElapsedTime().asSeconds() > 5.f){
        ll p = Rand(0,1); //p = 0;
        ll x,y = 0;
        if (p) x = Rand(0,400 - 70);
        else x = Rand(900,1300 - 70);
        bullet_icon.setPosition(x,y); bullet_icon.isDraw = 1;
        timerbullet.restart();
    }
    if (bullet_lf.isDraw){
        bullet_lf.moves(1.5,0);
        sf::Vector2f pos = bullet_lf.sprite.getPosition();
        if (pos.x > 1400) bullet_lf.isDraw = 0;
        if(checkplayertnt(player_lf, pos.x, pos.y))
        {
            player_lf_speed /= 1.7;
            timerspeedslow.restart();
            bullet_lf.isDraw = 0;
        }
        if(checkplayertnt(player_rg, pos.x, pos.y))
        {
            player_rg_speed /= 1.7;
            timerspeedslow.restart();
            bullet_lf.isDraw = 0;
        }
    }
    if (bullet_rg.isDraw){
        bullet_rg.moves(-1.5,0);
        sf::Vector2f pos = bullet_rg.sprite.getPosition();
        if (pos.x < -100) bullet_rg.isDraw = 0;
        if(checkplayertnt(player_lf, pos.x, pos.y))
        {
            player_lf_speed /= 1.7;
            timerspeedslow.restart();
            bullet_rg.isDraw = 0;
        }
        if(checkplayertnt(player_rg, pos.x, pos.y))
        {
            player_rg_speed /= 1.7;
            timerspeedslow.restart();
            bullet_rg.isDraw = 0;
        }
    }

    respawn();
}

void RenderGame(){

    background.drawObject();
    cloud.drawObject();
    ground_lf.drawObject(); ground_rg.drawObject();
    player_lf.drawObject();  player_rg.drawObject();
    box.drawObject();

    nuke.drawObject();
    explo_nuke.drawObject();

    tnt.drawObject();
    explo_tnt.drawObject();

    stuka_lf.drawObject();
    stuka_rg.drawObject();

    for(int i = 0; i < min(n, t/m); i ++)
    {
        boxes[i].isDraw = 1;
        boxes[i].drawObject();
    }
    air_icon.drawObject();
    drug.drawObject();
    bullet_lf.drawObject(); bullet_rg.drawObject(); bullet_icon.drawObject();
    if(t < n*m)t += 1;
    int now = 10-timer.getElapsedTime().asSeconds();
    if(now <= 0)ended = 1;
    cout << now <<" "<<ended<< endl;
    numberx.sprite.setTexture(texture_number[now/10]);
    numbery.sprite.setTexture(texture_number[now%10]);

    numberx.setPosition(400, 400);
    numbery.setPosition(480, 400);
    numberx.drawObject();
    numbery.drawObject();
}

void init_position(){
    player_rg.odirect = 1;
    player_rg.character = 1;
    player_lf.character = 1;
    background.setPosition(0, 0);
    ground_lf.setPosition(0,500); ground_rg.setPosition(900,500);
    player_lf.setPosition(0,420); player_rg.setPosition(1220,420);
    winner.setPosition(0, 0);
    box.setPosition(Rand(900,1220),0);

    for(int i = 0; i < n; i ++)
    {
        if(i&1)boxes[i].setPosition(Rand(0,320),0);
        else
            boxes[i].setPosition(Rand(900,1220),0);
        boxes[i].isDraw = 0;
    }

    cloud.setPosition(100, 100);

    air_icon.setPosition(Rand(370,850),Rand(200,460));

    stuka_lf.isDraw = stuka_rg.isDraw = nuke.isDraw = explo_nuke.isDraw = drug.isDraw = 0;
    explo_tnt.isDraw = 0; tnt.isDraw = 1;
    //bullet_lf.setPosition(0,430);
    bullet_lf.isDraw = bullet_rg.isDraw = bullet_icon.isDraw = 0;
    //bullet_icon.isDraw = 0;

}

void fend()
{
    cout << "ok" << endl;
    window.clear(sf::Color::Black);
    sf::Vector2f pos = box.sprite.getPosition();
    int cnt = (pos.x <= 400);
    for(int i = 0; i < min(t/m, n); i ++)
    {
        pos = boxes[i].sprite.getPosition();
        cnt += (pos.x <= 400.f);
    }
    winner.drawObject();
    font.loadFromFile("arial.ttf");
    if(cnt < 6)
    {
        sf::Text text("Player 1", font);
        text.setCharacterSize(100);
        text.setStyle(sf::Text::Bold);
        text.setFillColor(sf::Color::Red);
        text.setPosition(470, 200);
        window.draw(text);
    }
    else
    {
        sf::Text text("Player 2", font);
        text.setCharacterSize(100);
        text.setStyle(sf::Text::Bold);
        text.setFillColor(sf::Color::Red);
        text.setPosition(470, 200);
        window.draw(text);
    }
    window.display();
}
/*

Note
left_player: AD to move, W to jump, S to pick up, F to drop, space to throw
right_player: left right up down, K L to throw and pick up

*/
int main(){
    Loading(); init_position();
    while (window.isOpen()){
        sf::Event event;

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
                    if(timerdizzy_lf.getElapsedTime().asSeconds() > 6.0f)
                        player_lf.dizzy = 0;
                }
                else
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) player_lf.is_jump = 1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) player_lf.moves(-player_lf_speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
                        if (CanPick(player_lf,box)) player_lf.pickup(box);
                        else if (CanPick(player_lf,tnt)) player_lf.pickup(tnt);
                        else
                        {
                            for(int i = 0; i < min(n, t/m); i ++)
                                if(CanPick(player_lf, boxes[i]))player_lf.pickup(boxes[i]);
                        }
                    }


                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
                        if (player_lf.carry != NULL) player_lf.drop();

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) player_lf.moves(player_lf_speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) player_lf.throwStuff(1);
                }

                ///

                ///right player operation
                //if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
                if(player_rg.dizzy)
                {
                    if(timerdizzy_rg.getElapsedTime().asSeconds() > 6.0f)
                        player_rg.dizzy = 0;
                }
                else
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) player_rg.is_jump = 1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) player_rg.moves(-player_rg_speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
                        if (CanPick(player_rg,box)) player_rg.pickup(box);
                        else if (CanPick(player_rg,tnt)) player_rg.pickup(tnt);
                        else
                        {
                            for(int i = 0; i < min(n, t/m); i ++)
                            {
                                if(CanPick(player_rg, boxes[i]))
                                    player_rg.pickup(boxes[i]);
                            }
                        }
                    }



                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))
                        if (player_rg.carry != NULL) player_rg.drop();

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) player_rg.moves(player_rg_speed,0);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) player_rg.throwStuff(-1);
                }
                ///

                if(ended)break;
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

