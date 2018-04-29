#ifndef EJECTION_H
#define EJECTION_H

#include "circle.h"


class Ejection : public Circle
{
protected:
    int player;
    double speed;
    double angle;

public:
    explicit Ejection(int _id, double _x, double _y, double _radius, double _mass, int player) :
        Circle(_id, _x, _y, _radius, _mass),
        speed(0.0),
        angle(0.0),
        player(player)
    {
    }
    virtual ~Ejection() {}
public:
  void save_to(t_world_parsed&wp)const{
    qap_add_back(wp.earr).load_from(*this,false);
  }
public:
    double getA() const {
        return angle;
    }

    virtual bool is_my_eject(Circle *player) const {
        return this->player == player->getId();
    }

    virtual bool is_food() const {
        return true;
    }


public:
    void set_impulse(double _speed, double _angle) {
        speed = qAbs(_speed);
        angle = _angle;
    }

    bool move(int max_x, int max_y) {
        if (speed == 0.0) {
            return false;
        }
        double dx = speed * qCos(angle);
        double dy = speed * qSin(angle);

        double new_x = qMax(radius, qMin(max_x - radius, x + dx));
        bool changed = (x != new_x);
        x = new_x;

        double new_y = qMax(radius, qMin(max_y - radius, y + dy));
        changed |= (y != new_y);
        y = new_y;

        speed = qMax(0.0, speed - Constants::instance().VISCOSITY);
        return changed;
    }

};

typedef QVector<Ejection*> EjectionArray;

#endif // EJECTION_H
