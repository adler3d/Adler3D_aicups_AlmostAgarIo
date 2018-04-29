#ifndef PLAYER_H
#define PLAYER_H

#include "circle.h"
#include "ejection.h"


class Player : public Circle
{
public:
    bool is_fast=false;
    int fuse_timer=0;
    Player(){}
protected:
    //double speed=0, angle=0;
    vec2d v;
    vec2d dir;
    int fragmentId=0;
    double vision_radius=0;
    double cmd_x=0, cmd_y=0;

public:
    explicit Player(int _id, double _x, double _y, double _radius, double _mass, const int fId=0) :
        Circle(_id, _x, _y, _radius, _mass),
        is_fast(false),
        fuse_timer(0),
        fragmentId(fId),
        vision_radius(0),
        cmd_x(0), cmd_y(0)
    {
        if (fId > 0) {
            fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        }
    }

    virtual ~Player() {}
    /*
    QString id_to_str() const {
        if (fragmentId > 0) {
            return QString::number(id) + "." + QString::number(fragmentId);
        }
        return QString::number(id);
    }
    */
public:
  void save_to(t_world_parsed&wp)const{
    qap_add_back(wp.parr).load_from(*this,false);
  }
public:

    int get_fId() const {
        return fragmentId;
    }

    virtual bool is_player() const {
        return true;
    }

    QPair<double, double> get_direct() const {
        return QPair<double, double>(cmd_x, cmd_y);
    }

    double getVR() const {
        return vision_radius;
    }

public:
    void set_impulse(double new_speed, double new_angle) {
        v=Vec2dEx(new_angle,new_speed);
        is_fast = true;
    }

    void apply_viscosity(double usual_speed) {
        // если на этом тике не снизим скорость достаточно - летим дальше
        auto old_speed=v.Mag();auto speed=old_speed;
        if (speed - Constants::instance().VISCOSITY > usual_speed) {
            speed -= Constants::instance().VISCOSITY;
        } else {
            // иначе выставляем максимальную скорость и выходим из режима полёта
            speed = usual_speed;
            is_fast = false;
        }
        if(old_speed){v*=speed/old_speed;}else{
          v=dir.SetMag(speed);
        }
    }

    bool update_vision(int frag_cnt) {
        double new_vision;
        if (frag_cnt == 1) {
            new_vision = radius * VIS_FACTOR;
        }
        else {
            new_vision = radius * VIS_FACTOR_FR * qSqrt(frag_cnt);
        }
        if (vision_radius != new_vision) {
            vision_radius = new_vision;
            return true;
        }
        return false;
    }
    const vec2d get_pos()const{return vec2d(x,y);}
    static vec2d get_pos(const Circle&c){return vec2d(c.x,c.y);}
    inline vec2d get_vis_center()const{return get_pos(*this)+v.SetMag(VIS_SHIFT);}
    static vec2d get_vis_center(const vec2d&pos,const vec2d&v){return pos+v.SetMag(VIS_SHIFT);}
    static real get_vision_radius(double radius,int frag_cnt){return radius*(frag_cnt==1?VIS_FACTOR:VIS_FACTOR_FR)*qSqrt(frag_cnt);}
    bool can_see_by_adler(const Circle*circle){
      return get_vis_center().dist_to_point_less_that_r(get_pos(*circle),vision_radius+circle->radius);
    }
    bool can_see(const Circle *circle) {
        return can_see_by_adler(circle);/*
        double xVisionCenter = x + qCos(angle) * VIS_SHIFT;
        double yVisionCenter = y + qSin(angle) * VIS_SHIFT;
        double qdist = circle->calc_qdist(xVisionCenter, yVisionCenter);

        return (qdist < sqr(vision_radius + circle->radius));*/
    }

    real get_food_eat_radius()const{// if(dist<player.get_food_eat_radius())we_can_eat_it();
      auto R=FOOD_RADIUS;
      return +R-R*2*DIAM_EAT_FACTOR+radius;
    }

    double can_eat(Circle *food) const {
        if (food->is_player() && food->getId() == id) {
            return -INFINITY;
        }
        if (mass > food->getM() * MASS_EAT_FACTOR) { // eat anything
            auto R=food->radius;auto C=-R+R*2*DIAM_EAT_FACTOR;
            if(fabs(x-food->x)+C>radius)return -INFINITY;
            double dist = food->calc_dist(x, y);
            if (dist + C < radius) {
                return radius - dist;
            }
        }
        return -INFINITY;
    }

    void eat(Circle *food) {
        mass += food->getM();
    }

    bool can_burst(int yet_cnt) {
        if (mass < MIN_BURST_MASS * 2) {
            return false;
        }
        int frags_cnt = int(mass / MIN_BURST_MASS);
        if (frags_cnt > 1 && rest_fragments_count(yet_cnt) > 0) {
            return true;
        }
        return false;
    }

    void burst_on(Circle *virus) {
        double dy = y - virus->getY(), dx = x - virus->getX();
        
        double max_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);
        auto speed=v.Mag();
        v=vec2d(dx,dy).SetMag(speed<max_speed?max_speed:speed);

        mass += BURST_BONUS;
    }

    QVector<Player*> burst_now(int max_fId, int yet_cnt) {
        QVector<Player*> fragments;
        int new_frags_cnt = int(mass / MIN_BURST_MASS) - 1;

        new_frags_cnt = std::min(new_frags_cnt, rest_fragments_count(yet_cnt));

        double new_mass = mass / (new_frags_cnt + 1);
        double new_radius = mass2radius(new_mass);
        const auto angle=v.GetAng();
        for (int I = 0; I < new_frags_cnt; I++) {
            int new_fId = max_fId + I + 1;
            Player *new_fragment = new Player(id, x, y, new_radius, new_mass, new_fId);
            //new_fragment->set_color(color);
            fragments.push_back(new_fragment);

            double burst_angle = angle - BURST_ANGLE_SPECTRUM / 2 + I * BURST_ANGLE_SPECTRUM / new_frags_cnt;
            new_fragment->set_impulse(BURST_START_SPEED, burst_angle);
        }
        set_impulse(BURST_START_SPEED, angle + BURST_ANGLE_SPECTRUM / 2);

        fragmentId = max_fId + new_frags_cnt + 1;
        mass = new_mass;
        radius = new_radius;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        return fragments;
    }

    bool can_split(int yet_cnt) {

        if (rest_fragments_count(yet_cnt) > 0) {

            if (mass > MIN_SPLIT_MASS) {
                return true;
            }
        }
        return false;
    }

    Player *split_now(int max_fId) {
        double new_mass = mass / 2;
        double new_radius = mass2radius(new_mass);

        Player *new_player = new Player(id, x, y, new_radius, new_mass, max_fId + 1);
        //new_player->set_color(color);
        new_player->set_impulse(SPLIT_START_SPEED, v.GetAng());

        fragmentId = max_fId + 2;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        mass = new_mass;
        radius = new_radius;

        return new_player;
    }

    bool can_fuse(Player *frag) {
        double dist = frag->calc_dist(x, y);
        double nR = radius + frag->getR();

        return fuse_timer == 0 && frag->fuse_timer == 0 && dist <= nR;
    }
    bool can_fuse_v2(Player *frag) {
        double qdist = frag->calc_qdist(x, y);
        double d = radius + frag->getR();

        return qdist <= sqr(d);
    }

    void collisionCalc(Player *other) {
        if (is_fast || other->is_fast) { // do not collide splits
            return;
        }
        auto d=radius+other->radius;
        auto xdist=fabs(x-other->x);
        if(xdist>=d)return;
        double dist = this->calc_dist(other->x, other->y);
        if (dist >= d) {
            return;
        }

        // vector from centers
        double collisionVectorX = this->x - other->x;
        double collisionVectorY = this->y - other->y;
        // normalize to 1
        double vectorLen = qSqrt(collisionVectorX * collisionVectorX + collisionVectorY * collisionVectorY);
        if (vectorLen < 1e-9) { // collision object in same point??
            return;
        }
        collisionVectorX /= vectorLen;
        collisionVectorY /= vectorLen;

        double collisionForce = 1. - dist / (radius + other->radius);
        collisionForce *= collisionForce;
        collisionForce *= COLLISION_POWER;

        double sumMass = getM() + other->getM();
        // calc influence on us
        {
            double currPart = other->getM() / sumMass; // more influence on us if other bigger and vice versa

            double dx = v.x;
            double dy = v.y;
            dx += collisionForce * currPart * collisionVectorX;
            dy += collisionForce * currPart * collisionVectorY;
            this->v=vec2d(dx,dy);
        }

        // calc influence on other
        {
            double otherPart = getM() / sumMass;

            double dx = other->v.x;
            double dy = other->v.y;
            dx -= collisionForce * otherPart * collisionVectorX;
            dy -= collisionForce * otherPart * collisionVectorY;
            other->v=vec2d(dx,dy);
        }
    }

    void fusion(Player *frag) {
        double fragDX = frag->v.x;
        double fragDY = frag->v.y;
        double dX = v.x;
        double dY = v.y;
        double sumMass = mass + frag->mass;

        double fragInfluence = frag->mass / sumMass;
        double currInfluence = mass / sumMass;

        // center with both parts influence
        this->x = this->x * currInfluence + frag->x * fragInfluence;
        this->y = this->y * currInfluence + frag->y * fragInfluence;

        // new move vector with both parts influence
        dX = dX * currInfluence + fragDX * fragInfluence;
        dY = dY * currInfluence + fragDY * fragInfluence;

        // new angle and speed, based on vectors
        v=vec2d(dX,dY);

        mass += frag->getM();
    }

    bool can_eject() {
        return mass > MIN_EJECT_MASS;
    }

    Ejection *eject_now(int eject_id) {
        auto ep=vec2d(x,y)+v.SetMag(radius+1);
        double ex = ep.x;
        double ey = ep.y;

        Ejection *new_eject = new Ejection(eject_id, ex, ey, EJECT_RADIUS, EJECT_MASS, this->id);
        new_eject->set_impulse(EJECT_START_SPEED, v.GetAng());

        mass -= EJECT_MASS;
        radius = mass2radius(mass);
        return new_eject;
    }

    bool update_by_mass(int max_x, int max_y) {
        auto old_radius = radius;
        radius = mass2radius(mass);
        bool changed=old_radius!=radius;

        double new_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);
        if (v.SqrMag() > sqr(new_speed) && !is_fast) {
            v.SetMag(new_speed);
        }

        if (x - radius < 0) {
            x += (radius - x);
            changed = true;
        }
        if (y - radius < 0) {
            y += (radius - y);
            changed = true;
        }
        if (x + radius > max_x) {
            x -= (radius + x - max_x);
            changed = true;
        }
        if (y + radius > max_y) {
            y -= (radius + y - max_y);
            changed = true;
        }

        return changed;
    }

    void apply_direct(Direct direct, int max_x, int max_y) {
        direct.limit();
        cmd_x = direct.x; cmd_y = direct.y;
        if (is_fast) return;

        double speed_x = v.x;
        double speed_y = v.y;
        double max_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);

        double dy = direct.y - y, dx = direct.x - x;
        double dist = qSqrt(dx * dx + dy * dy);
        double ny = (dist > 0)? (dy / dist) : 0;
        double nx = (dist > 0)? (dx / dist) : 0;
        double k = Constants::instance().INERTION_FACTOR/mass;

        //v+=((direct.pos-pos).Norm()*max_speed-v)*k;
        speed_x += (nx * max_speed - speed_x)*k;
        speed_y += (ny * max_speed - speed_y)*k;

        v=vec2d(speed_x,speed_y);
        if(v.SqrMag()>sqr(max_speed))v.SetMag(max_speed);
    }

    bool move(int max_x, int max_y) {
        double rB = x + radius, lB = x - radius;
        double dB = y + radius, uB = y - radius;

        double dx = v.x;
        double dy = v.y;

        if(is_fast)dir=v;

        bool changed = false;
        if (rB + dx < max_x && lB + dx > 0) {
            x += dx;
            changed = true;
        }
        else {
            // долетаем до стенки
            double new_x = qMax(radius, qMin(max_x - radius, x + dx));
            changed |= (x != new_x);
            x = new_x;
            // зануляем проекцию скорости по dx
            v.x=0;
            if(is_fast)dir=vec2d(0,v.y?v.y:1);
        }
        if (dB + dy < max_y && uB + dy > 0) {
            y += dy;
            changed = true;
        }
        else {
            // долетаем до стенки
            double new_y = qMax(radius, qMin(max_y - radius, y + dy));
            changed |= (y != new_y);
            y = new_y;
            // зануляем проекцию скорости по dy
            v.y=0;
            if(is_fast)dir=vec2d(v.x?v.x:1,0);
        }

        if (is_fast) {
            double max_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);
            apply_viscosity(max_speed);
        }
        if (fuse_timer > 0) {
            fuse_timer--;
        }
        return changed;
    }

    static double get_max_speed(double mass){
      return Constants::instance().SPEED_FACTOR/qSqrt(mass);
    }

    static bool check_is_fast(double speed,double mass){
      //speed-=Constants::instance().VISCOSITY;
      return speed>get_max_speed(mass);
    }

    static int get_time_from_split_or_burst(double speed,double mass){
      auto dv=Constants::instance().VISCOSITY;
      auto min_spd=std::max<double>(speed,get_max_speed(mass));
      auto spd=std::max<double>(SPLIT_START_SPEED,BURST_START_SPEED);
      for(int i=0;;i++){
        spd-=dv;
        if(spd<min_spd)return std::min<int>(i,Constants::instance().TICKS_TIL_FUSION);
      }
      return 0;
    }
    
    bool can_shrink() {
        return mass > MIN_SHRINK_MASS;
    }

    void shrink_now() {
        mass -= ((mass - MIN_SHRINK_MASS) * SHRINK_FACTOR);
        radius = mass2radius(mass);
    }

    static double shrink_now(double mass){
      if(mass<=MIN_SHRINK_MASS)return mass;
      mass -= ((mass - MIN_SHRINK_MASS) * SHRINK_FACTOR);
      return mass;
    }

    static double shrink_sim(double mass,int beg_time,int end_time){
      for(int tick=beg_time;tick<end_time;tick++)
      {
        if(tick%SHRINK_EVERY_TICK!=0)continue;
        mass=shrink_now(mass);
      }
      return mass;
    }

public:
    static double mass2radius(double mass) {
        return PLAYER_RADIUS_FACTOR * std::sqrt(mass);
    }
public:
    /**
     * @param existingFragmentsCount число фрагментов игрока
     * @return максимально возможное число фрагментов, которое может дополнительно появиться у игрока в результате
     * взрыва / деления
     */
    static int rest_fragments_count(const int existingFragmentsCount) {
        return Constants::instance().MAX_FRAGS_CNT - existingFragmentsCount;
    }
};

typedef QVector<Player*> PlayerArray;

#endif // PLAYER_H
