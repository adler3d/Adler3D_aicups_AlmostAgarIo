#ifndef FOOD_H
#define FOOD_H

#include "circle.h"


class Food : public Circle{
public:
    Food(){};
    explicit Food(int _id, double _x, double _y, double _radius, double _mass) :
        Circle(_id, _x, _y, _radius, _mass)
    {
        //color = rand() % 14 + 4;
    }

    virtual ~Food() {}
    
    virtual bool is_food() const {
        return true;
    }
public:
  void save_to(t_world_parsed&wp)const{
    qap_add_back(wp.farr).load_from(*this,false);
  }
public:
};
typedef QVector<Food*> FoodArray;


#endif // FOOD_H
