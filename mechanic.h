#ifndef MECHANIC_H
#define MECHANIC_H

#include <functional>
#include <map>
#include <set>
#include <list>
#include <array>

#include "logger.h"
#include "entities/food.h"
#include "entities/virus.h"
#include "entities/player.h"
#include "entities/ejection.h"

#include "strategies/strategy.h"

typedef std::function<void(double, double)> AddFunc;
typedef std::function<Strategy*(Player*)> StrategyGet;

struct t_burst_hack{
  int pId=-1;
  int n=0;
  bool need_sim_without_burst=false;
};

struct t_score_hack{
  int pId=-1;
  int n=0;
  bool need_ignore_food=false;
};

struct Mechanic{
    int tick;
    int id_counter;

    t_burst_hack burst_hack;
    t_score_hack score_hack;

    typedef Player MechPlayer;
    t_foodmap*pfoodmap=nullptr;

    FoodArray food_array;
    EjectionArray eject_array;
    VirusArray virus_array;

    PlayerArray player_array;
    StrategyArray strategy_array;
    QMap<int, Direct> strategy_directs;
    QMap<int, int> player_scores;

    std::mt19937_64 rand;
    Mechanic(const Mechanic&){QapNoWay();}
public:
    explicit Mechanic() :
        tick(0),
        id_counter(1)
    {}

    virtual ~Mechanic() {
        clear_objects(false);
    }
public:
  double get_max_TTF()const{
    return Constants::instance().TICKS_TIL_FUSION;
  }
public:
  typedef QMap<int,Direct> QMap_int_Direct;
  typedef QMap<int,int> QMap_int_int;
  #define F(TYPE)static void clone(TYPE&dest,const TYPE&src){dest=src;}
    F(int);F(double);F(bool);F(QMap_int_Direct);F(QMap_int_int);
  #undef F
  template<class TYPE>
  static void clone(vector<TYPE*>&dest,vector<TYPE*>&src){
    dest.resize(src.size());
    for(int i=0;i<src.size();i++){
      auto&ex=dest[i];
      ex=new TYPE(*src[i]);
      //*ex=src[i];
    }
  }
  void fork_from(Mechanic&src)
  {
    #define F(FIELD)clone(this->FIELD,src.FIELD);
    F(tick);
    F(id_counter);
    F(food_array);
    F(eject_array);
    F(virus_array);
    F(player_array);
    F(strategy_array);
    F(strategy_directs);
    F(player_scores);
    #undef F
  }
public:
    void init_objects(const std::string &seed, const StrategyGet &get_strategy) {
        std::seed_seq seq(seed.begin(), seed.end());
        rand.seed(seq);

        std::array<uint, 1> simple_seeds;
        seq.generate(simple_seeds.begin(), simple_seeds.end());
        srand(simple_seeds[0]); // на всякий случай, если вдруг где-то когда-то будет использоваться обычный rand().
                                // он используется, например, в умолчальной стратегии
        //logger->init_file(QString::number(simple_seeds[0]), LOG_FILE, false);

        add_player(START_PLAYER_SETS, get_strategy);
        add_food(START_FOOD_SETS);
        add_virus(START_VIRUS_SETS);

    }

    void clear_objects(bool with_log=true) {
        tick = 0;

        id_counter = 1;
        for (Food *food : food_array) {
            if (food) delete food;
        }
        food_array.clear();

        for (Ejection *eject : eject_array) {
            if (eject) delete eject;
        }
        eject_array.clear();

        for (Virus *virus : virus_array) {
            if (virus) delete virus;
        }
        virus_array.clear();

        for (Player *player : player_array) {
            if (player) delete player;
        }
        player_array.clear();
        /*for (Strategy *strategy : strategy_array) {
            if (strategy) delete strategy;
        }*/
        strategy_array.clear();
    }

    int tickEvent_v2(bool inside_sim=false) {

        if(!inside_sim)apply_strategies();

        tick++;
        move_moveables();
        player_ejects();
        player_splits();

        if (tick % SHRINK_EVERY_TICK == 0) {
            shrink_players();
        }
        eat_all();
        fuse_players();
        burst_on_viruses();

        update_players_radius();

        split_viruses();

        if(!inside_sim)
        {
          if (tick % ADD_FOOD_DELAY == 0 && food_array.size() < MAX_GAME_FOOD) {
              add_food(ADD_FOOD_SETS);
          }
          if (tick % ADD_VIRUS_DELAY == 0 && virus_array.size() < MAX_GAME_VIRUS) {
              add_virus(ADD_VIRUS_SETS);
          }
        }

        if (tick % Constants::instance().BASE_TICK == 0) {
            //write_base_tick();
        }
        strategy_directs.clear();
        return tick;
    }

    bool known() const {
      QapNoWay();return false;
      /*
        QVector<int> livingIds;
        for (Player *player : player_array) {
            int pId = player->getId();
            if (! livingIds.contains(pId)) {
                livingIds.append(pId);
            }
        }
        if (livingIds.length() == 0) {
            return true;
        }
        else if (livingIds.length() == 1) {
            int living_score = player_scores[livingIds[0]];
            for (int pId : player_scores.keys()) {
                if (pId != livingIds[0] && player_scores[pId] >= living_score) {
                    return false;
                }
            }
            return true;
        }
        return false;*/
    }


public:

    bool is_space_empty(double _x, double _y, double _radius) const {
        for (Player *player : player_array) {
            if (player->is_intersected(_x, _y, _radius)) {
                return false;
            }
        }
        for (Virus *virus : virus_array) {
            if (virus->is_intersected(_x, _y, _radius)) {
                return false;
            }
        }
        return true;
    }

public:
#define append push_back
    void add_circular(int sets_cnt, double one_radius, const AddFunc &add_one) {
        double center_x = Constants::instance().GAME_WIDTH / 2, center_y = Constants::instance().GAME_HEIGHT / 2;
        for (int I = 0; I < sets_cnt; I++) {
            double _x = rand() % qCeil(center_x - 4 * one_radius) + 2 * one_radius;
            double _y = rand() % qCeil(center_y - 4 * one_radius) + 2 * one_radius;

            add_one(_x, _y);
            add_one(center_x + (center_x - _x), _y);
            add_one(center_x + (center_x - _x), center_y + (center_y - _y));
            add_one(_x, center_y + (center_y - _y));
        }
    }

    void add_food(int sets_cnt) {
        add_circular(sets_cnt, FOOD_RADIUS, [=] (double _x, double _y) {
            Food *new_food = new Food(id_counter, _x, _y, FOOD_RADIUS, Constants::instance().FOOD_MASS);
            food_array.append(new_food);
            id_counter++;
            if (tick % Constants::instance().BASE_TICK != 0) {
                //logger->write_add_cmd(tick, new_food);
            }
        });
    }

    void add_virus(int sets_cnt) {
        double rad = Constants::instance().VIRUS_RADIUS;
        add_circular(sets_cnt, rad, [=] (double _x, double _y) {
            if (! is_space_empty(_x, _y, rad)) {
                return;
            }
            Virus *new_virus = new Virus(id_counter, _x, _y, rad, VIRUS_MASS);
            virus_array.append(new_virus);
            id_counter++;
            if (tick % Constants::instance().BASE_TICK != 0) {
                //logger->write_add_cmd(tick, new_virus);
            }
        });
    }

    void add_player(int sets_cnt, const StrategyGet &get_strategy) {
        bool by_mouse = true;
        add_circular(sets_cnt, PLAYER_RADIUS, [=, &by_mouse] (double _x, double _y) {
            if (! is_space_empty(_x, _y, PLAYER_RADIUS)) {
                return;
            }
            Player *new_player = new Player(id_counter, _x, _y, PLAYER_RADIUS, PLAYER_MASS);
            player_array.append(new_player);
            new_player->update_by_mass(Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);

#ifdef LOCAL_RUNNER
            Strategy *new_strategy = get_strategy(new_player);
            strategy_array.append(new_strategy);
#endif

            player_scores[id_counter] = 0;
            id_counter++;
            if (tick % Constants::instance().BASE_TICK != 0) {
                //logger->write_add_cmd(tick, new_player);
            }
        });
    }

    template<class TYPE>
    static void delete_if_eq(vector<TYPE*>&arr,TYPE*ptr,bool dont=false){
        auto func=[&](TYPE*ex)
        {
          auto out=ptr==ex;
          if(!dont)if(out&&ex){
            delete ex;
          }
          return out;
        };
        clean_if(arr,func);
    }
    template<class TYPE>
    static void delete_if_eq(std::set<TYPE*>&arr,TYPE*ptr){
        auto it=arr.find(ptr);
        arr.erase(it);
        delete ptr;
    }

public:
    PlayerArray get_players_by_id_v2_deprecated_for_highload(int pId) const {
        PlayerArray result;
        for (Player *player : player_array) {
            if (player->getId() == pId) {
                if(!result.capacity())result.reserve(player_array.size());
                result.append(player);
            }
        }
        return result;
    }
    void get_players_by_id_v3(PlayerArray&out,int pId) const {
        out.clear();
        for (Player *player : player_array) {
            if (player->getId() == pId) {
                out.append(player);
            }
        }
    }
    template<class FUNC>
    void foreach_players(int pId,FUNC func) const {
        for (Player *player : player_array) {
            if (player->getId() == pId) {
                func(player);
            }
        }
    }
    PlayerArray get_players_by_id_deprecated(int pId) const {
        PlayerArray result;
        for (Player *player : player_array) {
            if (player->getId() == pId) {
                result.append(player);
            }
        }
        return result;
    }

    int get_fragments_cnt(int pId) const {
        int cnt = 0;
        for (Player *player : player_array) {
            if (player->getId() == pId) {
                cnt++;
            }
        }
        return cnt;
    }

    int get_max_fragment_id(int pId) const {
        int max_fId = 0;
        for (Player *player : player_array) {
            if (player->getId() == pId && max_fId < player->get_fId()) {
                max_fId = player->get_fId();
            }
        }
        return max_fId;
    }

    Strategy *get_strategy_by_id(int sId) const {
        for (Strategy *strategy : strategy_array) {
            if (strategy->getId() == sId) {
                return strategy;
            }
        }
        return NULL;
    }

    CircleArray get_visibles(const PlayerArray& for_them) const {
        // fog of war
        for (Player *player : player_array) {
            int frag_cnt = get_fragments_cnt(player->getId());
            bool updated = player->update_vision(frag_cnt);
            if (updated) {
                //logger->write_fog_for(tick, player);
            }
        }

        auto can_see = [&for_them](Circle* c){
            for (Player *fragment : for_them) {
                if (fragment->can_see(c)) {
                    return true;
                }
            }
            return false;
        };

        CircleArray visibles;
        for (Food *food : food_array) {
            if (can_see(food)) {
                visibles.append(food);
            }
        }
        for (Ejection *eject : eject_array) {
            if (can_see(eject)) {
                visibles.append(eject);
            }
        }
        auto pId = for_them.empty() ? -1 : for_them.front()->getId();
        for (Player *player : player_array) {
            if (player->getId() != pId && can_see(player)) {
                visibles.append(player);
            }
        }
        for (Virus *virus : virus_array) {
            visibles.append(virus);
        }
        return visibles;
    }

public:
    void apply_strategies() {
        for (Strategy *strategy : strategy_array) {
            int sId = strategy->getId();
            PlayerArray fragments = get_players_by_id_v2_deprecated_for_highload(sId);
            if (fragments.empty()) {
                continue;
            }
            CircleArray visibles = get_visibles(fragments);

            Direct direct = strategy->tickEvent(fragments, visibles);
//            //logger->write_direct(tick, sId, direct);

            apply_direct_for(sId, direct);
        }
    }

    void apply_direct_for(int sId, Direct direct) {
//        //logger->write_direct(tick, sId, direct);

        foreach_players(sId,[&](Player*frag){
          frag->apply_direct(direct, Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);
        });

        strategy_directs[sId]=direct;
    }

    bool can_player_do_split(int sId){
      bool out=false;
      int n=0;
      foreach_players(sId,[&](Player*frag){n++;});
      foreach_players(sId,[&](Player*frag){out=out||frag->can_split(n);});
      return out;
    }

    PlayerArray split_fragments_v2_buff;
    void split_fragments_v2(int sId) {
        auto&fragments=split_fragments_v2_buff;
        get_players_by_id_v3(fragments,sId);
        std::sort(fragments.begin(), fragments.end(), [] (const Player* lhs, const Player* rhs) {
            return
                std::make_tuple(lhs->getM(), lhs->get_fId()) >
                std::make_tuple(rhs->getM(), rhs->get_fId());
        });
        int fragments_count = fragments.size();

        for (Player *frag : fragments) {

            if (frag->can_split(fragments_count)) {
                int max_fId = get_max_fragment_id(frag->getId());
                //QString old_id = frag->id_to_str();

                Player *new_frag= frag->split_now(max_fId);
                player_array.push_back(new_frag);
                fragments_count++;

                //logger->write_add_cmd(tick, new_frag);
                //logger->write_change_mass_id(tick, old_id, frag);
            }
        }
    }

    void player_splits() {

        for (auto it = strategy_directs.begin(); it != strategy_directs.end(); it++) {
            const Direct& direct = it->second;

            if (direct.split) {
                const int player_id = it->first;
                split_fragments_v2(player_id);
            }
        }
    }
    PlayerArray player_ejects_fragments_buff;
    void player_ejects() {
        for (auto it = strategy_directs.begin(); it != strategy_directs.end(); it++) {
            int sId = (*it).first;
            Direct direct = (*it).second;
            if(direct.split || !direct.eject) {
                continue;
            }
            PlayerArray&fragments=player_ejects_fragments_buff;
            get_players_by_id_v3(fragments,sId);

            for (Player *frag : fragments) {
                if (frag->can_eject()) {
                    Ejection *new_eject = frag->eject_now(id_counter);
                    eject_array.append(new_eject);
                    id_counter++;

                    //logger->write_add_cmd(tick, new_eject);
                }
            }
        }
    }

    void eat_all() {
        auto nearest_player = [this] (Circle *circle) {
            Player *nearest_predator = NULL;
            double deeper_dist = -INFINITY;
            for (Player *predator : player_array) {
                double qdist = predator->can_eat(circle);
                if (qdist > deeper_dist) {
                    deeper_dist = qdist;
                    nearest_predator = predator;
                }
            }
            return nearest_predator;
        };
        auto nearest_virus = [this] (Ejection *eject) {
            Virus *nearest_predator = NULL;
            double deeper_dist = -INFINITY;
            for (Virus *predator : virus_array) {
                double qdist = predator->can_eat(eject);
                if (qdist > deeper_dist) {
                    deeper_dist = qdist;
                    nearest_predator = predator;
                }
            }
            return nearest_predator;
        };

        for (auto fit = food_array.begin(); fit != food_array.end();) {
            if (Player *eater = nearest_player(*fit)) {
                eater->eat(*fit);
                if(score_hack.need_ignore_food&&score_hack.pId==eater->id)
                {
                  score_hack.n+=SCORE_FOR_FOOD;
                }else{
                  player_scores[eater->getId()]+=SCORE_FOR_FOOD;
                }
                delete *fit;
                fit = food_array.erase(fit);
            } else {
                fit++;
            }
        }

        if(pfoodmap&&pfoodmap->need_use_inside_sim)
        {
          auto FOOD_MASS=Constants::instance().FOOD_MASS;
          QapAssert(food_array.empty());
          auto&arr=player_array;
          for(int i=0;i<arr.size();i++){
            auto&ex=arr[i];
            auto fer=ex->get_food_eat_radius();
            auto dn=pfoodmap->sim_eat(ex->get_pos(),fer,tick);
            if(!dn)continue;
            ex->mass+=dn*FOOD_MASS;
            player_scores[ex->id]+=SCORE_FOR_FOOD*dn;
            int gg=1;
          }
        }

        for (auto eit = eject_array.begin(); eit != eject_array.end(); ) {
            auto eject = *eit;
            if (Virus *eater = nearest_virus(eject)) {
                eater->eat(eject);
            } else if (Player *eater = nearest_player(eject)) {
                eater->eat(eject);
                if (!eject->is_my_eject(eater)) {
                    player_scores[eater->getId()] += SCORE_FOR_FOOD;
                }
            } else {
                eit++;
                continue;
            }

            //logger->write_kill_cmd(tick, eject);
            delete eject;
            eit = eject_array.erase(eit);
        }

        for (auto pit = player_array.begin(); pit != player_array.end(); ) {
            if(Player *eater = nearest_player(*pit)) {
                bool is_last = get_fragments_cnt((*pit)->getId()) == 1;
                eater->eat(*pit);
                player_scores[eater->getId()] += is_last? SCORE_FOR_LAST : SCORE_FOR_PLAYER;
                //logger->write_kill_cmd(tick, *pit);
                delete *pit;
                pit = player_array.erase(pit);
            } else {
                pit++;
            }
        }
    }

    void burst_on_viruses() { // TODO: improve target selection
        //PlayerArray targets = player_array; // WTF? this is not used

        auto nearest_to = [this] (Virus *virus) {
            double nearest_dist = INFINITY;
            Player *nearest_player = NULL;

            for (Player *player : player_array) {
                double qdist = virus->can_hurt(player);
                if (qdist < nearest_dist) {
                    int yet_cnt = get_fragments_cnt(player->getId());
                    if (player->can_burst(yet_cnt)) {
                        nearest_dist = qdist;
                        nearest_player = player;
                    }
                }
            }
            return nearest_player;
        };



        for (auto vit = virus_array.begin(); vit != virus_array.end(); ) {
            if (Player *player = nearest_to(*vit)) {
                if(burst_hack.need_sim_without_burst)if(player->id==burst_hack.pId){vit++;continue;}
                if(player->id==burst_hack.pId){
                  burst_hack.n++;
                }

                int yet_cnt = get_fragments_cnt(player->getId());
                int max_fId = get_max_fragment_id(player->getId());

                player->burst_on(*vit);
                player_scores[player->getId()] += SCORE_FOR_BURST;
                PlayerArray fragments = player->burst_now(max_fId, yet_cnt);
                player_array+=fragments;

                delete *vit;
                vit = virus_array.erase(vit);
            } else {
                vit++;
            }
        }
    }
    PlayerArray fuse_players_fragments_buff;
    vector<int> fuse_players_pids_buff;
    void fuse_players()
    {
      vector<int>&pids=fuse_players_pids_buff;int sId_offset=-1;
      pids.clear();
      if(!player_array.empty())
      {
        int min_id=QAP_MINVAL_ID_OF_VEC(player_array,+ex->id);int min_pid=player_array[min_id]->id;
        int max_id=QAP_MINVAL_ID_OF_VEC(player_array,-ex->id);int max_pid=player_array[max_id]->id;
        pids.resize(max_pid-min_pid+1,0);
        QAP_FOREACH(player_array,pids[ex->id-min_pid]++);
        sId_offset=min_pid;
      }

      PlayerArray fused_players;
      for(int i=0;i<pids.size();i++)
      {
        auto present=pids[i];
        if(!present)continue;
        int id=sId_offset+i;
        auto&fragments=fuse_players_fragments_buff;
        get_players_by_id_v3(fragments,id);
        std::sort(fragments.begin(),fragments.end(),[](const Player*a,const Player*b){return(
                std::make_tuple(a->mass,-a->fragmentId) >
                std::make_tuple(b->mass,-b->fragmentId)
        );});
        for(bool new_fusion_check=true;new_fusion_check;) 
        {
          new_fusion_check=false;
          for(int i=0;i<fragments.size();i++)
          {
            auto&a=fragments[i];
            if(!a||a->fuse_timer)continue;
            for(int j=i+1;j<fragments.size();j++)
            {
              auto&b=fragments[j];
              if(!a||a->fuse_timer)continue;
              if(!b||b->fuse_timer)continue;
              if(!a->can_fuse_v2(b))continue;
              a->fusion(b);
              fused_players.push_back(b);
              new_fusion_check=true;
              b=nullptr;
            }
          }
          if(!new_fusion_check)continue;
          for(int i=0;i<fragments.size();i++)
          {
            auto&ex=fragments[i];
            if(!ex)continue;
            fragments[i]->update_by_mass(Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);
          }
        }
      }
      for (Player *p : fused_players) {
          //logger->write_kill_cmd(tick, p);
          //delete p;
          delete_if_eq(player_array,p);
      }
    }
    
    PlayerArray move_moveables_fragments_buff;
    vector<int> move_moveables_pids_buff;
    void move_moveables() {
        Constants &ins = Constants::instance();
        for (Ejection *eject : eject_array) {
            bool changed = eject->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
            if (changed) {
                //logger->write_change_pos(tick, eject);
            }
        }
        for (Virus *virus : virus_array) {
            bool changed = virus->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
            if (changed) {
                //logger->write_change_pos(tick, virus);
            }
        }

        vector<int>&pids=move_moveables_pids_buff;int sId_offset=-1;
        pids.clear();
        if(!player_array.empty())
        {
          int min_id=QAP_MINVAL_ID_OF_VEC(player_array,+ex->id);int min_pid=player_array[min_id]->id;
          int max_id=QAP_MINVAL_ID_OF_VEC(player_array,-ex->id);int max_pid=player_array[max_id]->id;
          pids.resize(max_pid-min_pid+1,0);
          QAP_FOREACH(player_array,pids[ex->id-min_pid]++);
          sId_offset=min_pid;
        }

        for(int i=0;i<pids.size();i++)
        {
          auto present=pids[i];
          if(!present)continue;
          int sId=sId_offset+i;
          PlayerArray&fragments=move_moveables_fragments_buff;
          get_players_by_id_v3(fragments,sId);
          for (int i = 0; i != fragments.size(); ++i) {
              Player *curr = fragments[i];
              for (int j = i + 1; j < fragments.size(); ++j) {
                  curr->collisionCalc(fragments[j]);
              }
          }
        }

        for (Player *player : player_array) {
            bool changed = player->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
            if (changed) {
                //logger->write_change_pos(tick, player);
            }
        }
    }

    void update_players_radius() {
        for (Player *player : player_array) {
            bool changed = player->update_by_mass(Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);
            if (changed) {
                //logger->write_change_mass(tick, player);
            }
        }
    }

    void split_viruses() {
        VirusArray append_viruses;
        for (Virus *virus : virus_array) {
            if (virus->can_split()) {
                Virus *new_virus = virus->split_now(id_counter);
                //logger->write_add_cmd(tick, new_virus);
                append_viruses.append(new_virus);
                id_counter++;
            }
        }
        virus_array+=append_viruses;
    }

    void shrink_players() {
        for (Player *player : player_array) {
            if (player->can_shrink()) {
                player->shrink_now();
                //logger->write_change_mass(tick, player);
            }
        }
    }

    int get_score_for(int pId) const {
        return player_scores.at(pId);
    }

    QMap<int, int> get_scores() const {
        return player_scores;
    }
};

#endif // MECHANIC_H
