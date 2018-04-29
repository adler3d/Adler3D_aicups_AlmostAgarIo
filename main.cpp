// 2018.04.29 20:22:19 commit 2dac6bb4e96029384c6b608910aaa283de5d8826
//#undef Adler
#define QAP_DEBUG

#ifdef Adler
  //#define _ITERATOR_DEBUG_LEVEL 0
  #ifndef CLANG_QAPLITE
  #include "qaplite\QapLite.h"
  #endif
  #include "qaplite\TQapGameV2.inl"
  #include "qaplite\perf_sys.inl"
#else
  #ifdef WIN32
    #define NOMINMAX
    #include <Windows.h>
    class QapClock
    {
    public:
      INT64 freq,beg,tmp;
      bool run;
    public:
      QapClock(){QueryPerformanceFrequency((LARGE_INTEGER*)&freq);run=false;tmp=0;Start();}
      void Start(){QueryPerformanceCounter((LARGE_INTEGER*)&beg);run=true;}
      void Stop(){QueryPerformanceCounter((LARGE_INTEGER*)&tmp);run=false;tmp-=beg;}
      double Time(){if(run)QueryPerformanceCounter((LARGE_INTEGER*)&tmp);return run?double(tmp-beg)/double(freq):double(tmp)/double(freq);}
      double MS()
      {
        double d1000=1000.0;
        if(run)QueryPerformanceCounter((LARGE_INTEGER*)&tmp);
        if(run)return (double(tmp-beg)*d1000)/double(freq);
        if(!run)return (double(tmp)*d1000)/double(freq);
        return 0;
      }
    };
  #else
    #include <unistd.h>
    #include <sys/time.h>
    //struct  QapClock{double MS(){return 0;}};
    class QapClock
    {
    public:
      double beg,tmp;
      bool run;
    public:
      QapClock(){run=false;Start();}
      double em_perf_now(){
        timeval t;
        gettimeofday(&t,NULL);
        return (t.tv_sec*1e6+t.tv_usec)*1e-3;
      }
      void Start(){beg=em_perf_now();run=true;}
      void Stop(){tmp=em_perf_now();run=false;tmp-=beg;}
      double Time(){if(run)tmp=em_perf_now();return double(run?(tmp-beg):tmp)/1000.0;}
      double MS()
      {
        double d1000=1000.0;
        if(run)tmp=em_perf_now();
        if(run)return tmp-beg;
        if(!run)return tmp;
        //timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);
        return 0;
      }
    };
  #endif
  #define QAP_PERF(NAME)
  #define QAP_PERF_CODE(CODE){CODE;}
  #define NO_QAP_PERF_CODE(CODE){CODE;}
#endif

#define _ALLOW_KEYWORD_MACROS
#if(!defined(_DEBUG)&&!defined(Adler))
#ifndef QAP_MSVC
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wreorder"
#pragma warning(push,1)
#endif
#include <cstring>
#endif
#ifndef NOMINMAX
  #define NOMINMAX
#endif
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <stack>
#include <set>
#include <memory>
#include <thread>
#include <mutex>
#include <fstream>
#include <bitset>
using std::vector;
using std::string;
//#undef Adler

static bool unix_SaveFile(const string&FN,const string&mem)
{
  using namespace std;
  fstream f;
  f.open(FN.c_str(),ios::out|ios::binary);
  if(!f)return false;
  if(!mem.empty())f.write(&mem[0],mem.size());
  f.flush();
  f.close();
  return true;
};

#ifdef Adler
  #ifndef QAP_LITE_H
  #include <Windows.h>
  inline string IToS(const int&val){char c[16];_itoa_s(val,c,10);return string(c);}
  inline string FToS(const double&val){char c[64];if(abs(val)>1e9){_snprintf_s(c,32,32,"%e",val);}else{sprintf_s(c,"%f",val);}return string(c);}
  inline string FToS2(const double&val){char c[64];if(abs(val)>1e9){_snprintf_s(c,32,32,"%e",val);}else{sprintf_s(c,"%.2f",val);}return string(c);}
  static bool IsKeyDown(int vKey){int i=GetAsyncKeyState(vKey);return i<0;}
  #define KB_CODE(){auto mwta=game.getWizardMaxTurnAngle();if(IsKeyDown('Q'))move.setTurn(-mwta);if(IsKeyDown('E'))move.setTurn(+mwta);if(IsKeyDown('W'))move.setSpeed(+100);if(IsKeyDown('S'))move.setSpeed(-100);if(IsKeyDown('D'))move.setStrafeSpeed(+100);if(IsKeyDown('A'))move.setStrafeSpeed(-100);}
  static bool file_put_contents(const string&FN,const string&mem)
  {
    using namespace std;
    auto*f=fopen(FN.c_str(),"w+b");
    if(!f)return false;
    if(!mem.empty())fwrite(&mem[0],mem.size(),1,f);
    fclose(f);
    return true;
  };
  #endif
#else
  void KB_CODE(){}
  #define file_put_contents(...)(true)
  //static bool file_put_contents(const string&FN,const string&mem){return true;}
  string file_get_contents(const string&fn){return "";}
#endif
  
#ifndef QAP_LITE_H
#define QapDebugMsg(MSG){printf("QapDebugMsg :: %s:%i :: %s\n",__FILE__,__LINE__,string(MSG).c_str());fflush(stdout);}//__debugbreak();
#define QapAssert(COND)if(!(COND)){printf("QapAssert :: %s:%i :: %s\n",__FILE__,__LINE__,#COND);fflush(stdout);exit(0);}//__debugbreak();
#define QapNoWay(){printf("QapNoWay :: %s:%i\n",__FILE__,__LINE__);fflush(stdout);exit(0);}//__debugbreak();
#endif

#define DEF_PRO_SORT_BY_FIELD(sort_by_field,TYPE,FIELD)\
  struct t_help_struct_for_sort_vec_of_##TYPE##_by_##FIELD{\
    static int __cdecl cmp_func(const void*a,const void*b){return cmp(*(TYPE*)a,*(TYPE*)b);}\
    static int cmp(const TYPE&a,const TYPE&b){return a.FIELD-b.FIELD;}\
  };\
  static void sort_by_field(vector<TYPE>&arr){\
    if(arr.empty())return;\
    std::qsort(&arr[0],arr.size(),sizeof(TYPE),t_help_struct_for_sort_vec_of_##TYPE##_by_##FIELD::cmp_func);\
  }

#define PRO_FUNCGEN_GETP_BY_FIELD(rettype,getp,arr,field_type,field)\
  rettype*getp(field_type value)\
  {\
    rettype*p=nullptr;\
    for(int i=0;i<arr.size();i++){\
      auto&ex=arr[i];\
      if(ex.field!=value)continue;\
      QapAssert(!p);\
      p=&ex;\
    }\
    return p;\
  }

#define PRO_FUNCGEN_ADD_UNIQUE_OBJ_BY_FIELD_V2(rettype,adduni,arr,field_type,field)\
  rettype*adduni(field_type value)\
  {\
    rettype*p=nullptr;\
    for(int i=0;i<arr.size();i++){\
      auto&ex=arr[i];\
      if(ex.field!=value)continue;\
      QapAssert(!p);\
      p=&ex;\
    }\
    if(!p){p=&qap_add_back(arr);p->field=value;}\
    return p;\
  }

template<class TYPE,class FUNC>
int qap_minval_id_for_vec(vector<TYPE>&arr,FUNC func){
  if(arr.empty())return -1;
  decltype(func(arr[0],0)) val;int id=-1;
  for(int i=0;i<arr.size();i++){
    auto&ex=arr[i];
    auto tmp=func(ex,i);
    if(!i||tmp<val){
      val=tmp;id=i;
    }
  }
  return id;
}
template<class TYPE,class FUNC>
int qap_minval_id_for_vec(const vector<TYPE>&arr,FUNC func){
  if(arr.empty())return -1;
  decltype(func(arr[0],0)) val;int id=-1;
  for(int i=0;i<arr.size();i++){
    auto&ex=arr[i];
    auto tmp=func(ex,i);
    if(!i||tmp<val){
      val=tmp;id=i;
    }
  }
  return id;
}

template<class TYPE>
static void operator+=(vector<TYPE>&dest,const vector<TYPE>&arr){
  for(int i=0;i<arr.size();i++){
    dest.push_back(arr[i]);
  }
}

template<class TYPE>int qap_includes(const vector<TYPE>&arr,const TYPE&value){for(int i=0;i<arr.size();i++){if(arr[i]==value)return true;}return false;}

#define QAP_MINVAL_ID_OF_VEC(arr,code)qap_minval_id_for_vec(arr,[&](decltype(arr[0])&ex,int i){return code;})

template<class TYPE,class FUNC>void qap_foreach(TYPE&&arr,FUNC func){auto n=arr.size();for(int i=0;i<n;i++)func(arr[i],i);}
template<class TYPE,class FUNC>void qap_foreach(const TYPE&arr,FUNC func){auto n=arr.size();for(int i=0;i<n;i++)func(arr[i],i);}
#define QAP_FOREACH(arr,code)qap_foreach(arr,[&](decltype(arr[0])&ex,int i){code;})

#ifndef QAP_LITE_H
static double sqr(double x){return x*x;}

inline int SToI(const string&S){int i;sscanf(S.c_str(),"%i",&i);return i;};

inline string IToS(const int&v){return std::to_string(v);};
inline string FToS(const double&v){return std::to_string(v);};

template<class TYPE>void qap_sort(vector<TYPE>&arr){std::sort(arr.begin(),arr.end());}

template<class TYPE>
static bool qap_add_unique_val(vector<TYPE>&arr,const TYPE&value){
  if(qap_includes(arr,value))return false;
  arr.push_back(value);
  return true;
}

static vector<string> split(const string&s,const string&needle)
{
  vector<string> arr;
  if(s.empty())return arr;
  size_t p=0;
  for(;;){
    auto pos=s.find(needle,p);
    if(pos==std::string::npos){arr.push_back(s.substr(p));return arr;}
    arr.push_back(s.substr(p,pos-p));
    p=pos+needle.size();
  }
  return arr;
}
//-------------------------------------------//
static string join(const vector<string>&arr,const string&glue)
{
  string out;
  size_t c=0;
  size_t dc=glue.size();
  for(int i=0;i<arr.size();i++){if(i)c+=dc;c+=arr[i].size();}
  out.reserve(c);
  for(int i=0;i<arr.size();i++){if(i)out+=glue;out+=arr[i];}
  return out;
}

template<class TYPE>
static bool qap_find_val_once(const vector<TYPE>&arr,const TYPE&val){
  for(int i=0;i<arr.size();i++)if(val==arr[i])return true;
  return false;
}
template<class TYPE>
static bool qap_find_val_once(const std::set<TYPE>&arr,const TYPE&val){
  auto it=arr.find(val);
  return it!=arr.end();
}

template<typename TYPE,size_t COUNT>inline size_t lenof(TYPE(&)[COUNT]){return COUNT;}

template<class TYPE>static bool qap_check_id(const vector<TYPE>&arr,int id){return id>=0&&id<arr.size();}
template<class TYPE,class FUNC>void clean_if(vector<TYPE>&Arr,FUNC&&Pred){int last=0;for(int i=0;i<Arr.size();i++){auto&ex=Arr[i];if(Pred(ex))continue;if(last!=i){auto&ax=Arr[last];ax=std::move(ex);}last++;}if(last==Arr.size())return;Arr.resize(last);}

template<class TYPE>static TYPE&qap_add_back(vector<TYPE>&arr){arr.resize(arr.size()+1);return arr.back();}
template<typename TYPE>TYPE Sign(TYPE value){return (value>0)?TYPE(+1):TYPE(value<0?-1:0);}

typedef double real;const real Pi=3.14159265;const real Pi2=Pi*2;const real PiD2=Pi/2;const real PiD4=Pi/4;
template<class TYPE>inline TYPE Clamp(const TYPE&v,const TYPE&a,const TYPE&b){return std::max(a,std::min(v, b));}
template<typename TYPE>inline TYPE Lerp(const TYPE&A,const TYPE&B,const real&v){return A+(B-A)*v;}

class vec2d{
public:
  real x;real y;
  vec2d():x(0.0),y(0.0) {}
  vec2d(const real&x,const real&y):x(x),y(y) {}
  vec2d(const vec2d&v):x(v.x),y(v.y) {}
public:
  vec2d&operator=(const vec2d&v){x=v.x;y=v.y;return *this;}
  vec2d operator+()const{return *this;}
  vec2d operator-()const{return vec2d(-x,-y);}
  vec2d&operator+=(const vec2d&v){x+=v.x;y +=v.y;return *this;}
  vec2d&operator-=(const vec2d&v){x-=v.x; y-=v.y;return *this;}
  vec2d&operator*=(const real&f){x*=f;y*=f;return *this;}
  vec2d&operator/=(const real&f){x/=f;y/=f;return *this;}
public:
  vec2d Rot(const vec2d&OX)const{real M=OX.Mag();return vec2d(((x*+OX.x)+(y*OX.y))/M,((x*-OX.y)+(y*OX.x))/M);}
  vec2d UnRot(const vec2d&OX)const{real M=OX.Mag();if(M==0.0f){return vec2d(0,0);};return vec2d(((x*OX.x)+(y*-OX.y))/M,((x*OX.y)+(y*+OX.x))/M);}
  vec2d Ort()const{return vec2d(-y,x);}
  vec2d Norm()const{if((x==0)&&(y==0)){return vec2d(0,0);}return vec2d(x/this->Mag(),y/this->Mag());}
  vec2d SetMag(const real&val)const{return this->Norm().Mul(vec2d(val,val));}
  vec2d Mul(const vec2d&v)const{return vec2d(x*v.x,y*v.y);}
  vec2d Div(const vec2d&v)const{return vec2d(v.x!=0?x/v.x:x,v.y!=0?y/v.y:y);}
  real GetAng()const{return atan2(y,x);}
  real Mag()const{return sqrt(x*x+y*y);}
  real SqrMag()const{return x*x+y*y;}
public:
  real dist_to(const vec2d&p)const{return vec2d(p.x-x,p.y-y).Mag();}
  real sqr_dist_to(const vec2d&p)const{return vec2d(p.x-x,p.y-y).SqrMag();}
  bool dist_to_point_less_that_r(const vec2d&p,real r)const{return vec2d(p.x-x,p.y-y).SqrMag()<r*r;}
public:
  static vec2d min(const vec2d&a,const vec2d&b){return vec2d(std::min(a.x,b.x),std::min(a.y,b.y));}
  static vec2d max(const vec2d&a,const vec2d&b){return vec2d(std::max(a.x,b.x),std::max(a.y,b.y));}
  static void comin(vec2d&a,const vec2d&b){a=min(a,b);}
  static void comax(vec2d&a,const vec2d&b){a=max(a,b);}
  static vec2d sign(const vec2d&p){return vec2d(Sign(p.x),Sign(p.y));}
public:
  inline static real dot(const vec2d&a,const vec2d&b){return a.x*b.x+a.y*b.y;}
  inline static real cross(const vec2d&a,const vec2d&b){return a.x*b.y-a.y*b.x;}
  vec2d fabs()const{return vec2d(::fabs(x),::fabs(y));}
  real max()const{return std::max(x,y);}
  real min()const{return std::min(x,y);}
  //string dump()const{return "{\"X\":"+FToS(x)+",\"Y\":"+FToS(y)+"}";}
};
vec2d operator+(const vec2d&u,const vec2d&v){return vec2d(u.x+v.x,u.y+v.y);}
vec2d operator-(const vec2d&u,const vec2d&v){return vec2d(u.x-v.x,u.y-v.y);}
vec2d operator*(const vec2d&u,const real&v){return vec2d(u.x*v,u.y*v);}
vec2d operator*(const real&u,const vec2d&v){return vec2d(u*v.x,u*v.y);}
bool operator==(const vec2d&u,const vec2d&v){return (u.x==v.x)&&(u.y==v.y);}
bool operator!=(const vec2d&u,const vec2d&v){return !(u==v);}

inline vec2d Vec2dEx(const real&ang,const real&mag){return vec2d(cos(ang)*mag,sin(ang)*mag);}


class vec2i{
public:
public:
  typedef vec2i SelfClass;
public:
  int x;
  int y;
public:
public:
  vec2i():x(0),y(0) {}
  vec2i(int x,int y):x(x),y(y) {};
  friend vec2i operator*(int u,const vec2i&v)
  {
    return vec2i(u*v.x,u*v.y);
  }
  friend vec2i operator*(const vec2i&v,int u)
  {
    return vec2i(u*v.x,u*v.y);
  }
  friend vec2i operator/(const vec2i&v,int d)
  {
    return vec2i(v.x/d,v.y/d);
  }
  friend vec2i operator+(const vec2i&u,const vec2i&v)
  {
    return vec2i(u.x+v.x,u.y+v.y);
  }
  friend vec2i operator-(const vec2i&u,const vec2i&v)
  {
    return vec2i(u.x-v.x,u.y-v.y);
  }
  void operator+=(const vec2i&v)
  {
    x+=v.x;
    y+=v.y;
  }
  void operator-=(const vec2i&v)
  {
    x-=v.x;
    y-=v.y;
  }
  int SqrMag()
  {
    return x*x+y*y;
  }
  float Mag()
  {
    return sqrt(float(x*x+y*y));
  }
  operator vec2d()const
  {
    return vec2d(x,y);
  }
  vec2i operator+()const
  {
    return vec2i(+x,+y);
  }
  vec2i operator-()const
  {
    return vec2i(-x,-y);
  }
  friend bool operator==(const vec2i&u,const vec2i&v)
  {
    return (u.x==v.x)&&(u.y==v.y);
  }
  friend bool operator!=(const vec2i&u,const vec2i&v)
  {
    return (u.x!=v.x)||(u.y!=v.y);
  }
  static vec2i fromVec2d(const vec2d&v){return vec2i(int(v.x),int(v.y));}
  vec2i Ort()const{return vec2i(-y,x);}
  vec2i Mul(const vec2i&v)const{return vec2i(x*v.x,y*v.y);}
  static vec2i sign(const vec2i&p){return vec2i(Sign(p.x),Sign(p.y));}
  static vec2i sign(const vec2d&p){return vec2i(Sign(p.x),Sign(p.y));}
};

static vec2i tovec2i(const vec2d&p){return vec2i(p.x,p.y);}
#endif

template<class t_elem>
struct t_map{
  vector<t_elem> mem;
  int w;
  int h;
public:
  void fill(int value){for(int i=0;i<w*h;i++)mem[i]=value;}
public:
  static void set_to_def(t_elem&v){v.set_to_def();}
  t_elem&get(const vec2d&p){return get(p.x,p.y);}
  const t_elem&get(const vec2d&p)const{return get(p.x,p.y);}
  t_elem&get(const vec2i&p){return get(p.x,p.y);}
  const t_elem&get(const vec2i&p)const{return get(p.x,p.y);}
  t_elem&get_unsafe(int x,int y){return mem[x+y*w];}
  const t_elem&get_unsafe(int x,int y)const{return mem[x+y*w];}
  t_elem&get_unsafe(const vec2i&p){return mem[p.x+p.y*w];}
  const t_elem&get_unsafe(const vec2i&p)const{return mem[p.x+p.y*w];}
  t_elem&fail_id(){return sys_fail_id_const(*this);};
  template<class T>static t_elem&sys_fail_id_const(T&v){const T&ref=(const T&)v;return (t_elem&)ref.fail_id();};
  const t_elem&fail_id()const{static t_elem buff;set_to_def(buff);return buff;};
  t_elem&get(int x,int y){if(x<0||y<0)return fail_id();if(x<w&&y<h)return get_unsafe(x,y);return fail_id();}
  bool check(const vec2i&p)const{if(p.x<0||p.y<0||p.x+1>w||p.y+1>h)return false;return true;}
  const t_elem&get(int x,int y)const{if(x<0||y<0)return fail_id();if(x<w&&y<h)return get_unsafe(x,y);return fail_id();}
  int conv_vec_to_id(const vec2i&v)const{return v.x+v.y*w;}
  void init(vec2i wh){
    w=wh.x;h=wh.y;
    mem.resize(w*h);
  }
  vec2d get_wh(){return vec2d(w,h);}
};

#include "constants.h"

#undef GetObject

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
using namespace rapidjson;
using namespace std;


struct t_conf{
  bool need_init=true;
  #define LIST(F)\
  F(FOOD_MASS       )\
  F(GAME_HEIGHT     )\
  F(GAME_TICKS      )\
  F(GAME_WIDTH      )\
  F(INERTION_FACTOR )\
  F(MAX_FRAGS_CNT   )\
  F(SPEED_FACTOR    )\
  F(TICKS_TIL_FUSION)\
  F(VIRUS_RADIUS    )\
  F(VIRUS_SPLIT_MASS)\
  F(VISCOSITY       )\
  //===
  #define F(VAR)double VAR;
  LIST(F)
  #undef F
  void load(Document&d)
  {
    auto&c=Constants::instance();
    #define F(VAR)VAR=d[#VAR].GetDouble();c.VAR=VAR;
    LIST(F);
    #undef F
    need_init=false;
  }
  #undef LIST
};


struct t_cell{
  int call_id=-1;
  int beg_t=-1024*4;
  int t=0;
  real beg_food=0;
  real food_value=0;
  //
  int beg_food_n=0;
  int food_n=0;
  std::array<vec2d,7+10> food_pos;
  std::bitset<8*4> beg_food_flag;
  std::bitset<8*4> food_flag;
  void set_to_def(){call_id=-1;beg_t=0;t=0;beg_food=0;food_value=0;food_n=0;beg_food_n=0;}
  real&get_food_value(int sim_id,int tick,double food_spawn_speed){
    if(call_id!=sim_id){call_id=sim_id;t=beg_t;food_value=beg_food;food_n=beg_food_n;food_flag=beg_food_flag;}
    auto dt=tick-t;
    food_value+=dt*food_spawn_speed;
    t=tick;
    return food_value;
  }
  real get_beg_food_value(int tick,real food_spawn_speed)const{
    return beg_food_n+beg_food+(tick-beg_t)*food_spawn_speed;
  }
};
static const size_t t_cell_size=sizeof(t_cell);

struct t_foodmap{
  t_map<t_cell> map;
  int sim_id=0;
  real food_spawn_speed=0;
  real cell_area=0;
  real inv_cell_area=0;
  real inv_cell_size=0;
  vec2d offset;
  vec2d half_cell_size;
  bool need_use_inside_sim=false;
  bool need_use_foodvalue=false;
  void init(t_conf&conf,real cell_size=30){
    auto wh=vec2i(conf.GAME_WIDTH,conf.GAME_HEIGHT);
    auto map_wh=vec2i(wh.x/cell_size,wh.y/cell_size)+vec2i(0,0);
    auto map_area=wh.x*wh.y;
    map.init(map_wh);
    //cell_size=wh.x/map.w;
    inv_cell_size=1.0/cell_size;
    cell_area=sqr(cell_size);
    inv_cell_area=1.0/cell_area;
    food_spawn_speed=4.0*ADD_FOOD_SETS*cell_area/(ADD_FOOD_DELAY*map_area);
    half_cell_size=vec2d(1,1)*cell_size*0.5;
  }
  template<class Mechanic>
  real get_score(Mechanic&mech,int pId){
    typedef typename Mechanic::MechPlayer Player;
    auto&tick=mech.tick;
    real out=0;
    auto me_frag_cnt=mech.get_fragments_cnt(pId);
    mech.foreach_players(pId,[this,tick,&out,me_frag_cnt](Player*ptr){
      Player&ex=*ptr;
      auto pos=ex.get_pos();
      auto p=Player::get_vis_center(pos,ex.v);
      auto vr=Player::get_vision_radius(ex.radius,me_frag_cnt);
      out+=draw(p,vr,tick,true);
    });
    return out;
  }
  t_cell&get_cell(const vec2d&pos){
    auto p=pos*inv_cell_size;
    return map.get(p);
  }
  static vec2i tovec2i(const vec2d&p){return vec2i(p.x,p.y);}
  real draw(const vec2d&pos,real radius,int tick,bool inside_sim)
  {
    real out=0;
    auto half=vec2d(0.5,0.5);
    auto pc=pos*inv_cell_size;
    auto p=tovec2i(pc);
    auto r=radius*inv_cell_size;
    auto diag=sqrt(2);
    for(int y=p.y-r+1;y<=p.y+r;y++)for(int x=p.x-r+1;x<=p.x+r;x++){
      auto coords=vec2i(x,y);
      if(!map.check(coords))continue;
      auto cc=vec2d(coords)+half;
      //auto vertex=cc+vec2d::sign(cc-pc)/**diag*/*0.5;
      auto v=half;int ok=0;
      #define F()if((cc+v).dist_to_point_less_that_r(pc,r)){ok++;};v=v.Ort();
      F();F();F();F();
      #undef F
      if(ok!=4)continue;
      //dir.SqrMag()
      //if(!(vec2d(coords)+half).dist_to_point_less_that_r(pd,R+diag))continue;
      auto&c=map.get(coords);
      if(!inside_sim)
      {
        c.beg_t=tick;
        c.beg_food=0;
        c.beg_food_n=0;
        c.beg_food_flag.reset();
      }else{
        auto&fv=c.get_food_value(sim_id,tick,food_spawn_speed);
        out+=fv;
        fv=0;
      }
    }
    return out;
  }
  void draw_food(const vec2d&pos,int tick){
    auto&c=map.get(pos*inv_cell_size);
    if(bool need_use_food_pos=true){
      int&n=c.beg_food_n;
      for(int i=0;i<n;i++){
        if(c.food_pos[i]==pos){
          int gg=1;
          return;
        }
      }
      auto id=(n++)%c.food_pos.size();
      #ifdef Adler
      //QapAssert(n!=1);
      //QapAssert(n!=2);
      //QapAssert(n!=3);
      //QapAssert(n!=4);
      QapAssert(n!=5);
      QapAssert(n!=6);
      QapAssert(n!=7);
      QapAssert(n<=c.food_pos.size());
      #endif
      c.food_pos[id]=pos;
      c.beg_food_flag[id]=true;
      c.beg_food=0;
      n=std::min<int>(c.food_pos.size(),n);
    }else{
      QapNoWay();//deprecated
      c.beg_food=1;
    }
  }
  int sim_eat(const vec2d&pos,real food_eat_radius,int tick){
    auto half=vec2d(0.5,0.5);
    auto pc=pos*inv_cell_size;
    auto p=tovec2i(pc);
    auto R=food_eat_radius*inv_cell_size;
    auto r=R+1;
    auto diag=sqrt(2);
    int out=0;
    for(int y=p.y-r;y<=p.y+r;y++)for(int x=p.x-r;x<=p.x+r;x++){
      auto coords=vec2i(x,y);
      if(!map.check(coords))continue;
      auto&c=map.get(coords);
      if(!c.food_n)continue;
      QapAssert(c.food_n<=c.food_pos.size());
      auto&arr=c.food_pos;
      for(int i=0;i<c.beg_food_n;i++){
        if(!c.food_flag[i])continue;
        if(!arr[i].dist_to_point_less_that_r(pos,food_eat_radius))continue;
        c.food_flag[i]=false;
        out++;
        c.food_n--;
      }
      int gg=1;
    }
    return out;
  }
  #ifdef Adler
  void use_dump(TDataIO&IO,bool need_save){
    #define ADD(TYPE,NAME,VALUE)if(need_save)IO.save_as_pod(this->NAME);if(!need_save)IO.load_as_pod(this->NAME);
    //ADD(t_map<t_cell>,map,$);
    ADD(vector<t_cell>,map.mem,$);
    ADD(int,map.w,0);
    ADD(int,map.h,0);
    ADD(int,sim_id,0);
    ADD(real,food_spawn_speed,0);
    ADD(real,cell_area,0);
    ADD(real,inv_cell_area,0);
    ADD(real,inv_cell_size,0);
    ADD(vec2d,offset,$);
    ADD(vec2d,half_cell_size,$);
    #undef ADD
  }
  #endif
};

string dir="./";//"C:/Users/Adler/Desktop/miniraic/miniaicups/agario/local_runner_bin/win/";
string input_log_fn=dir+"inp_log.txt";
string full_game_fn=dir+"full_game.txt";

template<class JSON>
static void load_vec2d(vec2d&dest,JSON&d,bool speed=false){
  dest.x=d[speed?"SX":"X"].GetDouble();
  dest.y=d[speed?"SY":"Y"].GetDouble();
}

struct t_player{
  int id=-1;
  int fid=-1;
  vec2d pos;
  vec2d v;
  real r=0;
  real m=0;
  int TTF=0;
  bool me=false;
  //---
  int t=-1;
  int predicted_spawn_tick=0;
  int found_tick=-1;
  bool is_fast=false;
  //---
  template<class TYPE>
  void load(TYPE&d,bool mine){
    me=mine;
    auto Id=split(string(d["Id"].GetString()),".");
    id=SToI(Id[0]);
    fid=Id.size()<2?0:SToI(Id[1]);
    load_vec2d(pos,d);
    r=d["R"].GetDouble();
    m=d["M"].GetDouble();
    if(!mine)return;
    load_vec2d(v,d,true);
    if(d.HasMember("TTF"))TTF=d["TTF"].GetInt();
  }
  template<class TYPE>
  void load_from(const TYPE&d,bool mine){
    me=mine;
    id=d.id;
    fid=d.fragmentId;
    pos=vec2d(d.x,d.y);
    r=d.radius;
    m=d.mass;
    if(!mine)return;
    v=d.v;
    if(d.fuse_timer>0)TTF=d.fuse_timer;
  }
  void sync_with(t_player&ref){
    QapAssert(fid==ref.fid);
    QapAssert(id==ref.id);
    if(ref.me){*this=ref;return;}
    vec2d prev_pos=pos;
    *this=ref;
    v=pos-prev_pos;
    ref.v=v;
  }
};

struct t_food{
  vec2d pos;
  template<class TYPE>void load(TYPE&d){load_vec2d(pos,d);}
  template<class TYPE>void load_from(const TYPE&d,bool mine){pos=vec2d(d.x,d.y);}
};

struct t_eject{
  vec2d pos;
  int pId=0;
  template<class TYPE>void load(TYPE&d){load_vec2d(pos,d);if(d.HasMember("pId"))pId=d["pId"].GetInt();}
  template<class TYPE>void load_from(const TYPE&d,bool mine){pos=vec2d(d.x,d.y);pId=d.player;}
};

struct t_virus{
  vec2d pos;
  real m=0;
  template<class TYPE>void load(TYPE&d){load_vec2d(pos,d);m=d["M"].GetDouble();}
  template<class TYPE>void load_from(const TYPE&d,bool mine){pos=vec2d(d.x,d.y);m=d.mass;}
};

struct t_world_parsed{
  int tick=0;
  vector<t_player> parr;
  vector<t_food> farr;
  vector<t_eject> earr;
  vector<t_virus> varr;
  void load(Document&d,int tick)
  {
    this->tick=tick;
    auto mine=d["Mine"].GetArray();
    if(auto n=mine.Size())
    {
      for(int i=0;i<n;i++)
      {
        auto ex=mine[i].GetObject();
        qap_add_back(parr).load(ex,true);
      }
    }
    auto objects=d["Objects"].GetArray();
    if(auto n=objects.Size())
    {
      for(int i=0;i<n;i++)
      {
        auto ex=objects[i].GetObject();
        string type=ex["T"].GetString();
        if("F"==type)qap_add_back(farr).load(ex);
        if("E"==type)qap_add_back(earr).load(ex);
        if("V"==type)qap_add_back(varr).load(ex);
        if("P"==type)qap_add_back(parr).load(ex,false);
      }
    }
  }
  bool can_do_split(int sId){
    bool out=false;int n=0;int max_frags=Constants::instance().MAX_FRAGS_CNT;
    QAP_FOREACH(parr,if(ex.id==sId)n++);
    if(max_frags-n<=0)return false;
    QAP_FOREACH(parr,if(ex.id==sId)out=out||ex.m>MIN_SPLIT_MASS);
    return out;
  }
  vector<int> get_enemy_pIds(int our_id){
    vector<int> out;
    QAP_FOREACH(parr,if(ex.id!=our_id)qap_add_unique_val(out,ex.id));
    return out;
  }
  #ifdef Adler
  void use_dump(TDataIO&IO,bool need_save){
    #define ADD(TYPE,NAME,VALUE)if(need_save)IO.save_as_pod(this->NAME);if(!need_save)IO.load_as_pod(this->NAME);
    ADD(int,tick,0);
    ADD(vector<t_player>,parr,$);
    ADD(vector<t_food>,farr,$);
    ADD(vector<t_eject>,earr,$);
    ADD(vector<t_virus>,varr,$);
    #undef ADD
  }
  #endif
};

//#include "strategymodal.h"
#include "mechanic.h"
#include <iostream>

void load(t_world_parsed&out,const PlayerArray&FA,const CircleArray&CA)
{
  for(int i=0;i<FA.size();i++)
  {
    auto ex=FA[i];
    qap_add_back(out.parr).load_from(*ex,true);
  }
  for(int i=0;i<CA.size();i++)
  {
    auto ex=CA[i];
    ex->save_to(out);
  }
}

struct t_avg_real{
  real total=0;
  int n=0;
  real get(real value_when_n_is_zero=0){return n?total/n:value_when_n_is_zero;}
  void add(real v){total+=v;n++;}
};

real get_survive_probability(real er,real espd,real r,real spd,real ang,vec2d offset){return 0;}

static t_avg_real get_safety(Mechanic&mech,int pId){
  t_avg_real out;
  auto&arr=mech.player_array;
  for(int i=0;i<arr.size();i++){
    auto&a=arr[i];
    if(a->id==pId)continue;
    for(int i=0;i<arr.size();i++){
      auto&b=arr[i];
      if(b->id!=pId)continue;
      auto d=a->get_pos().dist_to(b->get_pos());
      QapAssert(a->mass);
      QapAssert(b->mass);
      out.add(d*b->mass/a->mass);
    }
  }
  return out;
}

struct t_frags_info{
  int n=0;
  real total_mass=0;
  real total_TFF=0;
  vec2d avg_pos;
};

static t_frags_info get_frags_info(Mechanic&mech,int pId){
  auto max_TTF=mech.get_max_TTF();
  t_frags_info out;
  auto&arr=mech.player_array;
  for(int i=0;i<arr.size();i++){
    auto&ex=arr[i];
    if(ex->id!=pId){
      int gg=1;
      continue;
    }
    out.avg_pos+=vec2d(ex->x,ex->y);
    out.total_mass+=ex->mass;
    out.total_TFF+=max_TTF-ex->fuse_timer;
    out.n++;
  }
  if(out.n)out.avg_pos*=1.0/out.n;
  return out;
}

struct t_score{
  real rank=-1;
  #define LIST(ADD)\
  ADD(real,total_score,0)\
  ADD(real,total_mass,0)\
  ADD(real,total_TFF,0)\
  ADD(real,total_safety,0)\
  ADD(real,total_food,0)\
  ADD(real,total_dist,0)\
  ADD(real,score,0)\
  ADD(real,mass,0)\
  ADD(real,TFF,0)\
  ADD(real,safety,0)\
  ADD(real,food,0)\
  ADD(real,dist,0)\
  //===
  //ADD(real,split_n,0)\
  //ADD(real,eject_n,0)\
  //ADD(real,total_avg_dist_to_enemys,0)\
  //ADD(real,total_avg_sqrdist_to_enemys,0)\
  //===
  #define F(TYPE,NAME,VALUE)TYPE NAME=VALUE;
  LIST(F)
  #undef F
  //
  int id=-1;
  real probability=1.0;
  t_frags_info bef;
  t_frags_info aft;
  real sources=0.0;
  void set_rank(real rank,int n){
    //real pos=(n-rank)/real(n);
    real N=1.25*n;
    probability=(N-rank)/N;
  }
  real total_mass_and_total_score()const{
    return total_mass+total_score;
  }
  real pos_diff()const{
    return bef.avg_pos.dist_to(aft.avg_pos);
  }
  bool operator<(const t_score&ref)const{
    #define MAJOR(SIGN,field)if(field!=ref.field)return SIGN field<SIGN ref.field;
    //MAJOR(-,aft.total_mass);
    //MAJOR(-,total_mass);
    MAJOR(-,total_mass_and_total_score());
    MAJOR(-,total_TFF);
    MAJOR(-,total_safety);
    MAJOR(-,total_score);
    MAJOR(-,total_food);
    MAJOR(-,total_dist);
    #undef MAJOR
    return id>ref.id;
  }
  bool almost_equal_to(const t_score&ref){
    t_score a=*this;
    t_score b=ref;
    std::swap(a.id,b.id);
    return ((*this)<ref)!=(a<b);
  }
  void add(const t_score&ref)
  {
    sources+=ref.probability;
    #define F(TYPE,NAME,VALUE)this->NAME+=ref.NAME*ref.probability;
    LIST(F);
    #undef F
  }
  t_score get_average()const
  {
    t_score out=*this;
    #define F(TYPE,NAME,VALUE)out.NAME=real(this->NAME)/sources;
    LIST(F);
    #undef F
    return out;
  }
  template<class FUNC>
  void foreach(FUNC func){
    #define F(TYPE,NAME,VALUE)func(#NAME,this->NAME);
    LIST(F);
    #undef F
  }
  string to_str(bool json=true,const string&glue=",")const{
    vector<string> out;
    #define F(TYPE,NAME,VALUE)qap_add_back(out)="\""+string(#NAME)+"\":"+std::to_string(this->NAME);
    F(int,id,-1);
    F(int,rank,-1);
    F(real,probability,1.0);
    F(real,sources,0.0);
    LIST(F);
    #undef F
    auto s=join(out,glue);
    return json?"{"+s+"}":s;
  }
  //t_score&set(int id){this->id=id;return *this;}
  #undef LIST
};

struct t_move{
  vec2d pos;
  bool split=false;// "Split": true,
  bool eject=false;// "Eject": true,
  t_move&set(vec2d p,bool s,bool e){split=s;eject=e;pos=p;return *this;}
  t_move operator=(const vec2d&ref){pos=ref;return *this;}
  t_move(){}
  t_move(const vec2d&ref){pos=ref;}
  operator Direct&(){return *(Direct*)(void*)this;}
};
struct t_plan_rec{
  int tick=-1;
  t_move move;
  t_plan_rec with_offset(int offset)const{t_plan_rec out=*this;out.tick+=offset;return out;}
};
struct t_plan{
  vector<t_plan_rec> arr;
  t_plan&add(int tick,const t_move&move){auto&b=qap_add_back(arr);b.tick=tick;b.move=move;return *this;}
  struct t_rec{bool ok;t_plan_rec pr;};
  t_rec get_rec(int tick)const{
    for(int i=0;i<arr.size();i++){auto&ex=arr[i];if(tick<=ex.tick){return {true,ex};}}
    return {false};
  }
  t_plan get_promoted()const
  {
    t_plan out;
    for(int i=0;i<arr.size();i++)
    {
      auto&ex=arr[i];
      if(ex.tick<1)continue;
      auto&b=qap_add_back(out.arr);
      b=ex;
      b.tick--;
    }
    return out;
  }
  bool try_save_move_and_promote(t_move&out,bool auto_promote=true,bool keep_length=true){
    auto rec=get_rec(0);
    if(!rec.ok)return false;
    out=rec.pr.move;
    if(auto_promote){
      if(!arr.empty())arr.back().tick++;
      *this=get_promoted();
    }
    return true;
  }
  operator bool()const{return !arr.empty();}
  static bool test(){
    t_plan plan;t_move a,b,c;a.pos.x=777;b.pos.x=888;c.pos.x=999;
    plan.add(0,a);
    QapAssert(plan.get_rec(0).ok);
    QapAssert(plan.get_rec(0).pr.move.pos.x==a.pos.x);
    QapAssert(!plan.get_rec(1).ok);
    plan.add(1,b);
    QapAssert(plan.get_rec(1).ok);
    QapAssert(plan.get_rec(0).pr.move.pos.x==a.pos.x);
    QapAssert(plan.get_rec(1).pr.move.pos.x==b.pos.x);
    QapAssert(!plan.get_rec(2).ok);
    plan.add(3,c);
    QapAssert(plan.get_rec(2).ok);
    QapAssert(plan.get_rec(3).ok);
    QapAssert(plan.get_rec(2).pr.move.pos.x==c.pos.x);
    QapAssert(plan.get_rec(3).pr.move.pos.x==c.pos.x);
    QapAssert(!plan.get_rec(4).ok);
    auto p=plan.get_promoted();
    QapAssert(p.get_rec(0).ok);
    QapAssert(p.get_rec(0).pr.move.pos.x==b.pos.x);
    QapAssert(p.get_rec(1).ok);
    QapAssert(p.get_rec(2).ok);
    QapAssert(!p.get_rec(3).ok);
    p=p.get_promoted();
    QapAssert(p.get_rec(0).ok);
    QapAssert(p.get_rec(1).ok);
    QapAssert(p.get_rec(0).pr.move.pos.x==c.pos.x);
    QapAssert(p.get_rec(1).pr.move.pos.x==c.pos.x);
    QapAssert(!p.get_rec(2).ok);
    p=p.get_promoted();
    QapAssert(p.get_rec(0).ok);
    QapAssert(p.get_rec(0).pr.move.pos.x==c.pos.x);
    QapAssert(!p.get_rec(1).ok);
    p=p.get_promoted();
    QapAssert(!p.get_rec(0).ok);
    int gg=1;
    return true;
  }
};

struct t_sim_env{
  t_score score;
  t_plan plan;
  Mechanic mech;
  int pId=0;
  int sim_limit=0;
};

struct t_plan_with_dbg{
  t_plan plan;
  string dbg;
  t_score score;
  vector<t_sim_env> envs;
  vector<vector<t_player>> players_logs;
  t_plan_with_dbg&err(const string&msg){dbg+="err:"+msg;return *this;}
  t_plan_with_dbg&set(const t_plan&ref){plan=ref;return *this;}
};

static bool test_passed=t_plan::test();

#ifdef Adler
//#define LOGREADMODE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef Document json;

struct t_hack_log{
  vector<string> arr;
  int counter=0;
};

typedef t_world_parsed t_world;

struct t_strategy{
public:
  bool need_init=true;
  std::mt19937_64 rand;
public:
  vector<string> inp_log;
  string cur_line;
  string conf_str;
  t_conf conf;
  t_hack_log HL;
public:
  t_world_parsed world;
  unique_ptr<t_plan_with_dbg> prev_cmd;
  t_plan cur_plan;
  //vec2d center,wh;
  t_foodmap foodmap;
public:
  bool pause=false;
  bool lock_step=false;
  vector<t_world_parsed> replay;
public:
  #ifdef Adler
  void go_dump_foodmap_and_world(int tick,bool need_save){
    auto fn="t_world_parsed."+IToS(tick)+".dump.bin";
    TDataIO IO;
    if(!need_save)IO.IO.LoadFile(fn);
    orig_world.use_dump(IO,need_save);
    world.use_dump(IO,need_save);
    foodmap.use_dump(IO,need_save);
    if( need_save)IO.save_as_pod(cur_plan.arr);if(!need_save)IO.load_as_pod(cur_plan.arr);
    if( need_save)IO.IO.SaveFile(fn);
  }
  #endif
public:
  ~t_strategy(){
    save_timing_before_exit();
    #ifdef Adler
    //file_put_contents(full_game_fn,inp_log);
    #endif
  }
  void save_timing_before_exit()
  {
    #if(defined(UNIX_WITH_FS)||defined(Adler))
    auto out=server_side.to_str("\n");
    unix_SaveFile("timing.txt",out);
    #endif
  }
  #ifdef LOGREADMODE
    bool cin_eof(){
      return !qap_check_id(HL.arr,HL.counter)||HL.arr[HL.counter].empty();
    }
    string parse_dump_log(string fn="188820_dump.log"){
      static string out;
      if(!out.empty())return out;
      auto log=file_get_contents(fn);
      log=StrReplace(log,"\r","");
      auto arr=split(log,"\n");
      vector<string> tmp;
      for(int i=0;i<arr.size();i++){
        auto&ex=arr[i];
        if(ex.empty()||'T'==ex[0]||ex.find("{\"Debug\":")!=string::npos)continue;
        tmp.push_back(ex);
      }
      out=join(tmp,"\n");
      file_put_contents("188820_dump.replay.log",out);
      //out=file_get_contents("debug.json");
      return out;
    }
    string readline(){
      static auto log=1?parse_dump_log():file_get_contents(input_log_fn);
      if(HL.arr.empty())HL.arr=split(log,"\n");
      if(!qap_check_id(HL.arr,HL.counter))return "";
      cur_line=HL.arr[HL.counter];
      HL.counter++;
      inp_log.push_back(cur_line);
      return cur_line;
    }
  #else
    bool cin_eof(){return cin.eof();}
    const string&readline(){
      static int counter=0;counter++;
      #ifdef Adler
      if(counter%300==0)file_put_contents("inp_log.txt",join(inp_log,"\n"));
      #endif
      cur_line.clear();
      std::cin>>cur_line;
      inp_log.push_back(cur_line);
      return cur_line;
    }
  #endif
  static Document unsafe_get_doc(const string&s){
    Document out;
    if(out.Parse<0>(s.c_str()).HasParseError()){
      QapDebugMsg("this line is not json:\nbeg\n"+s+"\nend");
      return out;
    }
    return out;
  }
  Document safe_get_doc(const string&s){
    Document out;
    if(out.Parse<0>(s.c_str()).HasParseError()){
      save_timing_before_exit();
      QapDebugMsg("this line is not json:\nbeg\n"+s+"\nend");
      return out;
    }
    return out;
  }
  void init()
  {
    #ifdef Adler
      /*
      auto log=file_get_contents(input_log_fn);
      auto arr=split(log,"}{");
      file_put_contents(input_log_fn,join(arr,"}\n{"));
      */
      static bool debug_hack=0;
      for(int i=0;debug_hack;i++)Sleep(16);
      if(bool need_confbin_test=false)
      {
        string confin=file_get_contents(dir+"conf.bin");
        if(confin.size()){
          t_conf&confbin=*(t_conf*)(void*)&confin[0];
          int gg=1;
        }
      }
    #endif
    auto conf_doc=safe_get_doc(conf_str=readline());
    
    conf.load(conf_doc);
    if(bool need_confout=false)
    {
      auto beg=(char*)(void*)&conf;
      auto end=beg+sizeof(conf);
      string confout(beg,end);
      file_put_contents("conf.bin",confout);
    }
    //
    //wh=vec2d(conf.GAME_WIDTH,conf.GAME_HEIGHT);center=wh*0.5;
  }
  void set_to_def()
  {
    rand.seed(0);
    need_init=true;
  }
  //-------------------------------------------------------- sync_world_with_storage ---------------------------------------------------------
public:
  #define PRO_FUNCGEN_GETP_BY_ID(rettype,getp,arr,t_id,id,id2id)\
    rettype*getp(t_id id){auto tmp=id2id[id];if(tmp<0)return nullptr;return &arr[tmp];}
  //
  struct t_id_and_fid{
    int id=0;
    int fid=0;
    void set(t_player&ref){id=ref.id;fid=ref.fid;}
  };
  struct t_storage{
    vector<t_player> arr;
    vector<int> fid2id;
    vector<int> fid2updated;
    PRO_FUNCGEN_GETP_BY_ID(t_player,fid2player,arr,int,fid,fid2id);
  }; 
  vector<t_storage> pid2storage;
  vector<t_id_and_fid> oldparr;
public:
  void sync_world_with_storage(){
    if(pid2storage.empty())pid2storage.resize(1024);
    if(world.parr.size()>1)
    {
      int gg=1;
    }
    QAP_FOREACH(world.parr,sync_with_storage_prepare(ex));
    int cleared=0;
    QAP_FOREACH(oldparr,auto&s=pid2storage[ex.id];if(s.fid2updated[ex.fid])return;cleared++;s.fid2id[ex.fid]=-1);
    if(cleared)
    {
      int gg=1;
    }
    oldparr.clear();
    QAP_FOREACH(world.parr,sync_with_storage(ex));
    if(bool need_upd_fid2updated=false)
    {
      QAP_FOREACH(oldparr,auto&s=pid2storage[ex.id];s.fid2updated[ex.fid]=0);
    }
    int gg=1;
  }
  t_storage&get_storage(int pid){
    auto&storage=pid2storage[pid];
    if(!storage.fid2id.empty())return storage;
    storage.fid2id.resize(1024*512,-1);
    storage.fid2updated.resize(1024*512,0);
    return storage;
  }
  void sync_with_storage_prepare(t_player&ex)
  {
    auto&storage=get_storage(ex.id);
    auto&s=pid2storage[ex.id];
    s.fid2updated[ex.fid]=1;
  }
  static string BToS(bool v){return v?"true":"false";}
  void sync_with_storage(t_player&ex)
  {
    int tick=world.tick+1; // +1 because of tick_event impl
    #ifdef Adler
      #define QAP_DBG(MSG)//{printf("%s\n",string(MSG).c_str());QapDebugMsg(MSG);}
    #else
      #define QAP_DBG(MSG)
    #endif
    auto&storage=get_storage(ex.id);
    qap_add_back(oldparr).set(ex);

    auto*ptr=storage.fid2player(ex.fid);
    if(!ptr)
    {
      storage.fid2id[ex.fid]=storage.arr.size();
      qap_add_back(storage.arr)=ex;
      ptr=storage.fid2player(ex.fid);
      if(!ex.me){
        int gg=1;
      }
    }
    //
    {
      auto prev_t=ptr->t;
      auto tdiff=tick-ptr->t;
      #if(1)
        if(prev_t<0){
          int gg=1;
          QAP_DBG("new player found: "+IToS(ptr->id)+"."+IToS(ptr->fid)+"\n");
          ex.found_tick=tick;
        }else{
          if(ex.me&&fabs(ex.TTF-conf.TICKS_TIL_FUSION)<1.01){
            ex.is_fast=Player::check_is_fast(ex.v.Mag(),ex.m);
            //QapDebugMsg(
            //  "ex.TTF=="+IToS(ex.TTF)+"\n"
            //  "ex.is_fast = "+BToS(ex.is_fast)+"\n"
            //  "Constants::instance().SPEED_FACTOR/qSqrt(ex.m) = "+FToS(Constants::instance().SPEED_FACTOR/qSqrt(ex.m))+"\n"
            //  "ex.v.Mag() = "+FToS(ex.v.Mag())
            //);
          }
          if(!ptr->me)
          {
            auto mdiff=ex.m-ptr->m;
            auto min_mass=Player::shrink_sim(ptr->m,ptr->t,tick)*0.9999;
            ex.predicted_spawn_tick=ptr->predicted_spawn_tick;
            if(ex.m<min_mass){
              ex.predicted_spawn_tick=ptr->t;// TTF = std::max<int>(0,conf.TICKS_TIL_FUSION-tdiff);
              ptr->predicted_spawn_tick=ex.predicted_spawn_tick;
              QAP_DBG("ex.m<min_mass for player: "+IToS(ptr->id)+"."+IToS(ptr->fid)+
                "\nmdiff = "+FToS(mdiff)+
                "\nex.m = "+FToS(ex.m)+
                "\nmin_mass = "+FToS(min_mass)+
                "\ntdiff = "+IToS(tdiff)+
                "\nptr->t = "+IToS(ptr->t)+
                "\ntick = "+IToS(tick)+
                "\nptr->found_tick = "+IToS(ptr->found_tick)+
                "\nex.predicted_spawn_tick = "+IToS(ex.predicted_spawn_tick)+
                "\n"
              );
            }
            ex.TTF=std::max<int>(0,conf.TICKS_TIL_FUSION-(tick-ptr->predicted_spawn_tick));
            ptr->TTF=ex.TTF;
            if(mdiff<0){
              //old and loose mass? or just new that eat a bit?
            }
            if(mdiff>0){
              //old eat something? or new eat a lot?
            }
            if(prev_t+1==tick){
              ex.found_tick=ptr->found_tick;
              QAP_DBG("upd for player: "+IToS(ptr->id)+"."+IToS(ptr->fid)+
                "\ntick = "+IToS(tick)+
                "\nex.found_tick = "+IToS(ex.found_tick)+
                "\nwatch_time = "+IToS(tick-ex.found_tick)+
                "\nTTF = "+IToS(ex.TTF)+
                "\nex.predicted_spawn_tick = "+IToS(ex.predicted_spawn_tick)+
                "\n"
              );
            }
            if(prev_t+1!=tick){
              ex.found_tick=tick;
              QAP_DBG("old player found: "+IToS(ptr->id)+"."+IToS(ptr->fid)+"  from tick = "+IToS(prev_t)+
                "\ntick = "+IToS(tick)+
                "\npredicted_spawn_tick = "+IToS(ex.predicted_spawn_tick)+
                "\nTTF = "+IToS(ex.TTF)+
                "\nex.predicted_spawn_tick = "+IToS(ex.predicted_spawn_tick)+
                "\n"
              );
            }
          }
          int gg=1;
        }
      #endif
      ptr->sync_with(ex);
      ptr->t=tick;
      ex.is_fast=Player::check_is_fast(ex.v.Mag(),ex.m);
      ptr->is_fast=ex.is_fast;
      auto watch_time=tick-ex.found_tick;
      if(!ex.me){
        int gg=1;
      }
      if(!ex.me&&ex.is_fast&&watch_time==2)
      {
        int split_tick=tick-Player::get_time_from_split_or_burst(ex.v.Mag(),ex.m);
        QAP_DBG("watch_time==2 for player: "+IToS(ptr->id)+"."+IToS(ptr->fid)+
          "\ntick = "+IToS(tick)+
          "\nex.found_tick = "+IToS(ex.found_tick)+
          "\nwatch_time = "+IToS(watch_time)+
          "\nsplit_tick = "+IToS(split_tick)+
          ""
        );
        ptr->predicted_spawn_tick=split_tick-1; // -1 because it is better
      }
      int gg=1;
      return;
    }
    int gg=1;
    #undef QAP_DBG
  }
public:
  //-------------------------------------------------------- BEGIN ---------------------------------------------------------
  t_move qap_run_step()
  {
    QapClock iter_clock;
    unique_ptr<t_plan_with_dbg> up_cmd;
    up_cmd.reset(new t_plan_with_dbg());
    on_tick(*up_cmd);
    auto out=qap_run_apply(std::move(up_cmd));
    total_ms+=iter_clock.MS();
    return out;
  }
  t_move qap_run_apply(unique_ptr<t_plan_with_dbg>&&up_cmd)
  {
    t_move out;
    cur_plan=up_cmd->plan;
    cur_plan.try_save_move_and_promote(out,true);
    if(dont_split_we_see_enemy_with_unknow_speed){
      up_cmd->dbg+=":@@@:{dont_split_we_see_enemy_with_unknow_speed:true}";out.split=false;
    }
    prev_cmd=std::move(up_cmd);
    return out;
  }
  //-------------------------------------------------------- RUN_BEG ---------------------------------------------------------
  real total_ms=0;
  void run()
  {
    real prev_iter_ms=0;
    QapClock iter_clock;
    set_to_def();
    string data;
    for(int tick=0;!cin_eof();tick++)
    {
      auto&s=readline();iter_clock.Stop();iter_clock.Start();
      auto parsed=safe_get_doc(s);
      world=t_world_parsed();
      world.load(parsed,tick);
      QapClock clock;
      unique_ptr<t_plan_with_dbg> up_cmd;up_cmd.reset(new t_plan_with_dbg());auto&cmd=*up_cmd;
      on_tick(cmd);
      auto on_tick_ms=clock.MS();
      if(0==tick)cmd.dbg="/* 2018.04.29 20:22:19 commit 2dac6bb4e96029384c6b608910aaa283de5d8826 */ "+cmd.dbg+" // conf_str = "+conf_str;
      vector<string> timers;
      qap_add_back(timers)="TICK:"+std::to_string(tick);
      qap_add_back(timers)="TOT:"+std::to_string(total_ms);
      qap_add_back(timers)="ITR:"+std::to_string(prev_iter_ms);
      qap_add_back(timers)="ONT:"+std::to_string(on_tick_ms);
      cmd.dbg=jobj(join(timers,","))+":@@@:"+cmd.dbg;
      {
        auto cur_move=qap_run_apply(std::move(up_cmd));
        string s=dump(cur_move,string2json(cmd.dbg));
        prev_iter_ms=iter_clock.MS();total_ms+=prev_iter_ms;
        fprintf(stdout,"%s\n",s.c_str());
      }
    }
  }
  //-------------------------------------------------------- RUN_END ---------------------------------------------------------
  #define toStr std::to_string
  struct t_server_side_rec{
    int ppd=0;
    int dirs=0;
    real ms_per_dir=0;
    string to_str()const{return "{ppd:"+toStr(ppd)+",dirs:"+toStr(dirs)+",ms:"+toStr(ms_per_dir)+"}";}
  };
  struct t_server_side_item{
    int players=11;
    real ms=36.0/8; // time_of(sim(env_with_11_players)) = 36.0/8 ms; // 4.5 ms/dir;
    real total_ms=0;
    int n=0;
    real get_average()const{return !n?0:total_ms/n;}
    bool operator<(const t_server_side_item&ref)const{return players<ref.players;}
    string to_str()const{return "f("+toStr(players)+","+toStr(get_average())+");";}
  };
  #undef toStr
  struct t_server_side{
    real TL=21.0;
    t_server_side_rec last;
    vector<t_server_side_item> items;
    vector<t_server_side_rec> arr;
    PRO_FUNCGEN_ADD_UNIQUE_OBJ_BY_FIELD_V2(t_server_side_item,add_item,items,int,players);
    t_server_side(){fixed_config();}
    void add(int dirs,int ppd,real ms){
      auto&b=*add_item(ppd);
      b.n++;
      b.total_ms+=ms/dirs;
      auto&r=qap_add_back(arr);
      r.dirs=dirs;
      r.ppd=ppd;
      r.ms_per_dir=ms/dirs;
      last=r;
    }
    string to_str(const char*glue=""){
      vector<string> out;
      qap_sort(items);
      for(int i=0;i<items.size();i++){
        auto&ex=items[i];
        if(!ex.n)continue;
        qap_add_back(out)=ex.to_str();
      }
      return join(out,glue);
    }
    void clear()
    {
      items.clear();arr.clear();
    }
    void fixed_config()
    {//return;
      auto f=[this](int ppd,real ms){auto&out=*add_item(ppd);out.players=ppd;out.ms=ms;return out;};
      f(1,0.053670);f(2,0.093260);f(3,0.122391);f(4,0.170327);f(5,0.250247);f(6,0.231170);f(7,0.418099);f(8,0.492174);f(9,0.572157);f(10,0.545299);f(11,0.741503);f(12,0.771849);f(13,0.848097);f(14,0.759347);f(15,0.563756);f(16,0.759870);f(17,0.689572);
      /*1,0.051749);f(2,0.080696*/f(3,0.147570);f(4,0.201055);/*5,0.222384*/f(6,0.269286);f(7,0.429097);/*8,0.437488);f(9,0.380044);f(10,0.487755);f(11,0.469488);f(12,0.614803);f(13,0.677455);f(14,0.639959*/f(15,0.864199);f(16,0.786589);f(17,0.914799);f(18,0.959808);f(19,0.935922);f(20,0.967625);f(21,0.982549);f(22,0.915562);f(23,1.061054);f(24,1.076198);f(25,0.951227);f(26,0.761149);f(27,1.060425);f(28,1.070305);f(29,0.668880);
    }
    real ppd2ms(int ppd){return add_item(ppd)->ms;}
    real get_dirs_per_tick(int players_per_dir)
    {
      auto ms_per_dir=ppd2ms(players_per_dir);
      return TL/ms_per_dir;
    }
    real get_dirs_per_tick_v2(int players_per_dir,real TL_koef)
    {
      auto ms_per_dir=ppd2ms(players_per_dir);
      return TL*TL_koef/ms_per_dir;
    }
  };
  t_server_side server_side;
  //-------------------------------------------------------- ON_TICK_BEG ---------------------------------------------------------
  static const int sim_limit=64;
  vector<t_plan> movdirs;
  vector<t_plan> passive_movdirs;
  bool evolution=false;
  bool evolution_rot=false;
  bool evolution_split=false;
  bool smart_enemy=false;
  static t_plan with_split(const t_plan&src,int tick=0){
    if(!src.arr.size())return src;
    auto&arr=src.arr;
    t_plan b;
    if(tick==1){
      int gg=1;
    }
    if(tick){
      int gg=1;
    }else{
      int gg=1;
    }
    tick--;
    bool added=false;
    for(int i=0;i<arr.size();i++){
      auto&ex=arr[i];
      if(ex.tick<tick){qap_add_back(b.arr)=ex;continue;}
      if(!added){
        if(tick>=0)
        {
          qap_add_back(b.arr)=ex;
          #ifdef Adler
          QapAssert(!b.arr.back().move.split);
          #endif
          b.arr.back().tick=tick;
        }
        qap_add_back(b.arr)=ex;
        b.arr.back().move.split=true;
        b.arr.back().tick=tick+1;
        added=true;
      }
      qap_add_back(b.arr)=ex.with_offset(+2);
    }
    return b;
  }
  static vector<t_plan> with_split(const vector<t_plan>&arr,int tick=0){
    vector<t_plan> out;
    QAP_FOREACH(arr,qap_add_back(out)=with_split(ex,tick));
    return out;
  }
  typedef vector<t_plan> t_movdirs;
  static t_movdirs build_movdirs_for_enemy_v3(int sim_limit,bool without_splits=true,int dirs=12)
  {
    t_movdirs base=get_movdirs(dirs*2,false,false,sim_limit,600);
    if(without_splits)
    {
      t_movdirs out;
      QAP_FOREACH(base,if(0==i%2)qap_add_back(out)=ex);
      qap_add_back(out).add(sim_limit,t_move().set(vec2d(0,0),false,false));
      return out;
    }
    t_movdirs out;
    QAP_FOREACH(base,if(1==i%2)qap_add_back(out)=with_split(ex,1==i%4?1:sim_limit/2));
    qap_add_back(out).add(sim_limit,t_move().set(vec2d(0,0),false,false));
    return out;
  }
  void init_movdirs(int version=0)
  {
    typedef vector<t_plan> t_plans;
    if(movdirs.size())return;
    auto v11_2=[](){
      t_plans out;
      out+=get_movdirs_bow(8,sim_limit,700);
      out+=get_movdirs_with_kink_v0(8,sim_limit,700);
      qap_add_back(out).add(sim_limit,t_move().set(vec2d(0,0),false,false));
      return out;
    };
    if(6==version){movdirs=v11_2();return;}
    auto v15_1=[](){
      t_plans out;
      out+=get_movdirs_v3(8,false,false,sim_limit,600);
      out+=get_movdirs(19,false,false,sim_limit,600);
      out+=with_split(out,sim_limit/4);
      return out;
    };
    if(7==version){movdirs=v15_1();return;}
    QapNoWay();
  }
  struct t_sim_v3_brains{
    vector<int> pIds;
    vector<t_plan> plans;
    void apply_direct_for(Mechanic&mech)
    {
      for(int i=0;i<plans.size();i++)
      {
        auto&ex=plans[i];
        auto&epid=pIds[i];
        if(epid<0)continue;
        auto rec=ex.get_rec(i);QapAssert(rec.ok);auto&m=rec.pr.move;
        mech.apply_direct_for(epid,m);
      }
    }
  };
  bool always_add_curplan=true;
  bool use_center_plan=false;
  bool use_center_plan_when_no_enemy=false;
  bool use_curplan_when_no_enemy=false;
  bool use_passive_movdirs_when_no_enemy=false;
  bool use_smart_v4_hardness_when_enemy=false;
  bool need_ignore_foodvalue_when_enemy=false;
  bool need_turn_off_foodmap_after_7k=false;
  bool dont_split_we_see_enemy_with_unknow_speed=false;
  t_world orig_world;
  void on_tick(t_plan_with_dbg&out)
  {
    int iter=world.tick;
    orig_world=world;
    if(bool need_restore_orig_world=false){
      world=orig_world;
    }
    auto&w=world;
    sync_world_with_storage();
    dont_split_we_see_enemy_with_unknow_speed=false;
    if(bool need_remove_enemy_with_unknown_speed_from_w_parr=true)
    {
      auto tick=iter+1; // +1 because of tick_event impl
      //auto is_new=[tick](t_player&ex){auto watch_time=tick-ex.found_tick;return watch_time<=0;};
      //real mass[2]={0,0};real n[2]={0,0};
      //QAP_FOREACH(w.parr,mass[ex.me]+=ex.m;n[ex.me]++;);
      //QAP_FOREACH(w.parr,if(!ex.me&&is_new(ex))ex.v=vec2d(1e-6,1e-6););
      //clean_if(w.parr,[tick,&is_new](t_player&p){if(p.me)return false;return is_new(p);});
      clean_if(w.parr,[tick](t_player&p){if(p.me)return false;auto watch_time=tick-p.found_tick;return watch_time<=0;});
      dont_split_we_see_enemy_with_unknow_speed=w.parr.size()!=orig_world.parr.size();
      int gg=1;
    }

    t_player*p=nullptr;t_player*enemy=nullptr;int me_frag_cnt=0;
    QAP_FOREACH(w.parr,if(ex.me){me_frag_cnt++;if(!p)p=&ex;};if(!enemy&&!ex.me)enemy=&ex;);

    QapAssert(!conf.need_init);
    if(foodmap.map.mem.empty())
    {
      foodmap.init(conf);
      QapAssert(!foodmap.map.mem.empty());
    }
    //#ifdef Adler
    //if(IsKeyDown('N'))Sleep(100);
    //if(pause)for(int i=0;!IsKeyDown(VK_RIGHT)&&pause;i++)Sleep(16);if(IsKeyDown(VK_RIGHT))Sleep(100);
    //#endif

    if(need_turn_off_foodmap_after_7k&&iter>6300){
      foodmap.need_use_foodvalue=false;
      need_ignore_foodvalue_when_enemy=false;
    }
    if(need_ignore_foodvalue_when_enemy){
      foodmap.need_use_foodvalue=!enemy;
    }
    if(foodmap.need_use_foodvalue||need_ignore_foodvalue_when_enemy)
    {
      QAP_FOREACH(world.parr,/*if(ex.me)*/foodmap.draw(Player::get_vis_center(ex.pos,ex.v),Player::get_vision_radius(ex.r,me_frag_cnt),iter,false));
      //QAP_FOREACH(world.parr,/*if(ex.me)*/foodmap.draw(ex.pos,ex.r,iter));
    }
    if(foodmap.need_use_inside_sim)
    {
      QAP_FOREACH(world.farr,foodmap.draw_food(ex.pos,iter));
      world.farr.clear();
    }

    #ifdef LOGREADMODE
      if(0)if(world.tick<1500){
        out.err("skip in debug");
        return;
      }
      //if(world.tick%100==0)fprintf(stderr,"\nworld.tick = %i\n",world.tick);
      if(world.tick>2220)fprintf(stderr,"\n%s\n",cur_line.c_str());
    #endif

    if(!p)
    {
      out.err("look loke inp.Mine.Size()==0");
      #if(!(defined(Adler)||defined(DONT_EXIT)))
        static int fail_counter=0;
        fail_counter++;
        if(fail_counter==10)exit(0);
      #endif
      return;
    }
    auto our_id=p->id;
    vec2d avg_pos;int n=0;
    QAP_FOREACH(w.parr,if(!ex.me)return;avg_pos+=ex.pos;n++;);avg_pos*=(1.0/n);
    {
      if(bool need_auto_enable_cheap_algo=true)
      {
        auto time_per_tick=conf.GAME_TICKS?real(538*1000.0)/25000:real(134.0*1000.0)/7500;
        auto time_per_game=time_per_tick*conf.GAME_TICKS;
        auto time_consumed=real(int(total_ms))+1.0;
        auto time_left=time_per_game-time_consumed;
        auto ticks_left=(conf.GAME_TICKS+10)-world.tick;
        auto allowed_ms_per_tick=time_left/ticks_left;
      
        bool use_cheap_algo=allowed_ms_per_tick<server_side.TL;
        out.dbg+=":@@@:{int_total_ms:"+std::to_string(int(total_ms))+",allowed_ms_per_tick:"+std::to_string(allowed_ms_per_tick)+",use_cheap_algo:"+string(use_cheap_algo?"true":"false")+"}";

        if(use_cheap_algo)
        {
          auto&app=*this;
          app.smart_enemy=false;
          app.smart_v4=false;
          app.use_smart_v4_hardness_when_enemy=false;
          app.use_passive_movdirs_when_no_enemy=false;
          movdirs=passive_movdirs;
        }
      }
      init_movdirs();
      if(use_smart_v4_hardness_when_enemy)
      {
        QapAssert(smart_v4);
        QapAssert(always_add_curplan);
        QapAssert(use_passive_movdirs_when_no_enemy);
      }
      auto cur_movdirs=use_passive_movdirs_when_no_enemy&&!enemy?passive_movdirs:movdirs;

      auto ppd=w.parr.size();
      vector<t_plan> curset;
      auto dirs_per_tick=server_side.get_dirs_per_tick(ppd);
      if(use_smart_v4_hardness_when_enemy)if(smart_v4&&enemy){
        auto hardness=movdirs_for_enemy*1.70;
        dirs_per_tick*=2.0*(iter<7200?1.2:1.0); // allow use more cpu time per tick when enemy present.
        dirs_per_tick/=hardness;
        if(dirs_per_tick<2)dirs_per_tick=2;
      }
      auto dpt=std::max<int>(1,dirs_per_tick);
      int load_divider=std::max<int>(1,cur_movdirs.size()/dpt);
      int subset=iter%load_divider;
      QAP_FOREACH(cur_movdirs,if(i%load_divider==subset)qap_add_back(curset)=addbase(ex,avg_pos));
      //QAP_FOREACH(movdirs2,qap_add_back(curset)=addbase(ex,avg_pos));
      if(use_center_plan||(use_center_plan_when_no_enemy&&!enemy))qap_add_back(curset).add(sim_limit,t_move().set(avg_pos,false,false));
      if(always_add_curplan||(use_curplan_when_no_enemy&&!enemy))
      {
        if(cur_plan.arr.size())qap_add_back(curset)=cur_plan;
      }
      if(evolution_rot||evolution)
      {
        if(cur_plan.arr.size()){
          auto&b=qap_add_back(curset);b=cur_plan;auto base=avg_pos;auto rot=Vec2dEx(+Pi2/128,1);
          for(int i=0;i<b.arr.size();i++){
            auto&p=b.arr[i].move.pos;
            p=base+(p-base).UnRot(rot);
          }
        }
        if(cur_plan.arr.size()){
          auto&b=qap_add_back(curset);b=cur_plan;auto base=avg_pos;auto rot=Vec2dEx(-Pi2/128,1);
          for(int i=0;i<b.arr.size();i++){
            auto&p=b.arr[i].move.pos;
            p=base+(p-base).UnRot(rot);
          }
        }
      }
      if(evolution_split||evolution)
      {
        if(cur_plan.arr.size()&&w.can_do_split(our_id)){
          auto&b=qap_add_back(curset)=with_split(cur_plan);
        }
      }
      if(evolution)
      {
        //if(cur_plan.arr.size()){
        //  auto&b=qap_add_back(curset);
        //  b.add(0,t_move().set(cur_plan.arr[0].move.pos,false,true));
        //  for(int i=0;i<cur_plan.arr.size();i++){
        //    qap_add_back(b.arr)=cur_plan.arr[i].with_offset(1);
        //  }
        //}
        if(cur_plan.arr.size()){
          auto&b=qap_add_back(curset);
          for(int i=0;i<cur_plan.arr.size();i++){
            qap_add_back(b.arr)=cur_plan.arr[i].with_offset(+1);
          }
        }
        if(cur_plan.arr.size()){
          auto&b=qap_add_back(curset);
          auto base=avg_pos;
          auto pos=cur_plan.arr[0].move.pos;
          b.add(0,t_move().set(base-(pos-base),true,false));
          for(int i=0;i<cur_plan.arr.size();i++){
            qap_add_back(b.arr)=cur_plan.arr[i].with_offset(1);
          }
        }
        if(cur_plan.arr.size()){
          auto&b=qap_add_back(curset);
          b=cur_plan;
          b.arr.back().tick=sim_limit;
          b=b.get_promoted();
        }
        real scale_koef=(64.0-16.0)/64.0;real inv_scale_koef=1.0/scale_koef;
        if(cur_plan.arr.size()){
          auto&b=qap_add_back(curset);b=cur_plan;auto base=avg_pos;
          for(int i=0;i<b.arr.size();i++){
            auto&p=b.arr[i].move.pos;
            p=base+(p-base)*scale_koef;
            break;
          }
        }
        if(cur_plan.arr.size()){
          auto&b=qap_add_back(curset);b=cur_plan;auto base=avg_pos;
          for(int i=0;i<b.arr.size();i++){
            auto&p=b.arr[i].move.pos;
            p=base+(p-base)*inv_scale_koef;
            break;
          }
        }
      }
      QapClock server_side_clock;
      auto&envs=out.envs;
      envs.resize(curset.size());
      int fmsi_bef=foodmap.sim_id;
      for(int i=0;i<curset.size();i++)
      {
        auto&env=envs[i];
        env.score.id=i;
        env.pId=our_id;
        env.sim_limit=sim_limit;
        env.plan=curset[i];
        sim_auto(env,nullptr,nullptr);
      }
      auto sim_id_diff=foodmap.sim_id-fmsi_bef;
      out.dbg+=":@@@:{sim_id_diff:"+IToS(sim_id_diff)+",curset:"+IToS(curset.size())+"}";
      server_side.add(sim_id_diff,ppd,server_side_clock.MS());
      if(server_side.last.ppd)
      {
        bool full=iter%1000==999||iter==(7500-2)||iter==(25000-2)||iter==(40000-2);
        out.dbg+=":@@@:"+(
          full?server_side.to_str():server_side.last.to_str()
        );
        auto ms_allowed=server_side.ppd2ms(ppd);
        out.dbg+=":@@@:{ppd2ms:"+std::to_string(ms_allowed)+",'cur/fixed':"+std::to_string(!ms_allowed?-1:server_side.last.ms_per_dir/ms_allowed)+"}";
      }
      auto id=QAP_MINVAL_ID_OF_VEC(envs,ex.score);
      if(id<0)
      {
        out.err("curset.empty();");
        return;
      }
      auto&win=envs[id];
      #ifdef LOGREADMODE
        if(world.tick>=8289)
        {
          vector<t_score> arr;
          vector<t_score> arr_reversed;
          QAP_FOREACH(envs,qap_add_back(arr)=ex.score);
          qap_sort(arr);
          arr_reversed=arr;reverse(arr_reversed);
          vector<string> dbg;
          QAP_FOREACH(arr,qap_add_back(dbg)=jobj(
            ex.to_str(false)
            +","+jk("move",jq(join(split(dump(envs[ex.id].plan.get_rec(0).pr.move,""),"\""),"")))
            +","+jk("total_mass_and_total_score",(ex.total_mass_and_total_score()))
          ));
          string gg=join(dbg,"\n");
          file_put_contents("dbg.out.txt",gg);
          volatile int gg2=1;
          QapDebugMsg("tick=="+IToS(world.tick));
        }
      #endif
      out.plan=win.plan;
      out.score=win.score;
      #ifdef Adler
      if(bool need_full_debug_info=true){
        //ok
        t_sim_env env;
        env.score.id=win.score.id;
        env.pId=our_id;
        env.sim_limit=sim_limit;
        env.plan=win.plan;
        sim_auto(env,&out.players_logs,nullptr);
      }else{
        envs.clear(); //pointless but...
      }
      #endif
      //mwd.dbg=jobj("s:"+std::to_string(score[id]));
      int gg=1;
    }
  }
  bool smart_v4=false;
  void sim_auto(t_sim_env&env,vector<vector<t_player>>*plog=nullptr,t_sim_v3_brains*pbrains=nullptr)
  {
    if(smart_v4)
    {
      QapAssert(!pbrains);
      sim_v4_full_v2(env,plog);
      return;
    }
    if(smart_enemy){
      QapNoWay();
      return;
    }else{
      return sim_v5(env,plog);
    }
  }
  //-------------------------------------------------------- ON_TICK_END ---------------------------------------------------------
  static vector<vec2d> get_movdirs(int dirs,real mag=100){
    vector<vec2d> id2dir;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++)qap_add_back(id2dir)=Vec2dEx(i*inv_dirs*Pi2,mag);
    return id2dir;
  }
  static t_plan addbase(const t_plan&src,vec2d base){
    auto out=src;
    for(int i=0;i<out.arr.size();i++){
      auto&ex=out.arr[i];
      ex.move.pos+=base;
    }
    return out;
  }
  static vector<t_plan> get_movdirs(int dirs,bool EJECT,bool SPLIT,int sim_limit,real mag){
    vector<t_plan> out;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++){
      auto dir=Vec2dEx(i*inv_dirs*Pi2,mag);
      qap_add_back(out).add(sim_limit,t_move().set(dir,false,false));
      if(SPLIT)qap_add_back(out).add(sim_limit,t_move().set(dir,true,false));
      if(EJECT)qap_add_back(out).add(sim_limit,t_move().set(dir,false,true));
      if(EJECT&&SPLIT)qap_add_back(out).add(sim_limit,t_move().set(dir,true,true));
    }
    return out;
  }
  static vector<t_plan> get_movdirs_v2(int dirs,bool EJECT,bool SPLIT,int sim_limit,real mag){
    vector<t_plan> out;int half=sim_limit/2;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++){
      auto dir=Vec2dEx(i*inv_dirs*Pi2,mag);
      for(int j=0;j<dirs;j++)if(i!=j){
        auto dir2=Vec2dEx(j*inv_dirs*Pi2,mag);
        qap_add_back(out).add(half,t_move().set(dir,false,false)).add(sim_limit,t_move().set(dir2,false,false));
      }
    }
    return out;
  }
  static vector<t_plan> get_movdirs_v3(int dirs,bool EJECT,bool SPLIT,int sim_limit,real mag){
    vector<t_plan> out;int T1=sim_limit/3;int T2=sim_limit*2/3;int T3=sim_limit;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++)
    {
      auto dir=Vec2dEx(i*inv_dirs*Pi2,mag);
      for(int j=0;j<dirs;j++)if(i!=j)
      {
        auto dir2=Vec2dEx(j*inv_dirs*Pi2,mag);
        for(int k=0;k<dirs;k++)if(j!=k)
        {
          auto dir3=Vec2dEx(k*inv_dirs*Pi2,mag);
          qap_add_back(out)
            .add(T1,t_move().set(dir,SPLIT,EJECT))
            .add(T2,t_move().set(dir2,SPLIT,EJECT))
            .add(T3,t_move().set(dir3,SPLIT,EJECT));
        }
      }
    }
    return out;
  }
  static vector<t_plan> get_movdirs_v4_beg(int dirs,bool EJECT,bool SPLIT,int sim_limit,real mag){
    vector<t_plan> out;int T1=0;int T2=sim_limit;int T3=sim_limit;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++)
    {
      auto dir=Vec2dEx(i*inv_dirs*Pi2,mag);
      for(int j=0;j<dirs;j++)if(i!=j)
      {
        auto dir2=Vec2dEx(j*inv_dirs*Pi2,mag);
        qap_add_back(out)
          .add(T1,t_move().set(dir,SPLIT,EJECT))
          .add(T2,t_move().set(dir+dir2,false,false));
      }
    }
    return out;
  }
  static vector<t_plan> get_movdirs_v4_mid(int dirs,bool EJECT,bool SPLIT,int sim_limit,real mag){
    vector<t_plan> out;int T1=0;int T2=sim_limit/2;int T3=sim_limit;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++)
    {
      auto dir=Vec2dEx(i*inv_dirs*Pi2,mag);
      for(int j=0;j<dirs;j++)if(i!=j)
      {
        auto dir2=Vec2dEx(j*inv_dirs*Pi2,mag);
        qap_add_back(out)
          .add(T1,t_move().set(dir,false,false))
          .add(T2,t_move().set(dir+dir2,SPLIT,EJECT))
          .add(T3,t_move().set(dir+dir2,false,false));
      }
    }
    return out;
  }
  static vector<t_plan> get_movdirs_with_kink(int dirs,int sim_limit,real mag,int k=1){
    vector<t_plan> out;int T0=0;int T1=1;int T2=sim_limit/2;int T3=sim_limit;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++)
    {
      auto dir0=Vec2dEx(i*inv_dirs*Pi2,mag);
      for(int j=0;j<2;j++)
      {
        auto dir1=Vec2dEx((i+(j?-k:+k))*inv_dirs*Pi2,mag);
        auto dir2=Vec2dEx((i+(j?-k*2:+k*2))*inv_dirs*Pi2,mag);
        auto dir3=Vec2dEx((i+(j?+k:-k))*inv_dirs*Pi2,mag);
        qap_add_back(out)
          .add(T0,t_move().set(dir0,false,false))
          .add(T1,t_move().set(dir1,false,false))
          .add(T2,t_move().set(dir2,false,false))
          .add(T3,t_move().set(dir2+dir3,false,false));
      }
    }
    return out;
  }
  static vector<t_plan> get_movdirs_with_kink_v0(int dirs,int sim_limit,real mag,real k=0.5){
    vector<t_plan> out;int T0=0;int T1=sim_limit;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++)
    {
      auto dir0=Vec2dEx(i*inv_dirs*Pi2,mag);
      for(int j=0;j<2;j++)
      {
        auto dir1=Vec2dEx((i+(j?+k:-k))*inv_dirs*Pi2,mag);
        qap_add_back(out)
          .add(T0,t_move().set(dir1,false,false))
          .add(T1,t_move().set(dir0,false,false));
      }
    }
    return out;
  }
  static vector<t_plan> get_movdirs_vibro(int dirs,int sim_limit,real mag,int dt=8){
    vector<t_plan> out;auto dmag=mag*dt/sim_limit;
    real inv_dirs=1.0/dirs;
    for(int i=0;i<dirs;i++)
    {
      auto dir0=Vec2dEx((i+(0.2))*inv_dirs*Pi2,dmag);
      auto dir1=Vec2dEx((i-(0.2))*inv_dirs*Pi2,dmag);
      auto pos=vec2d(0,0);
      auto&b=qap_add_back(out);
      for(int i=1;i<=sim_limit/dt;i++){
        pos+=i%2?dir0:dir1;
        b.add(i*dt,t_move().set(pos,false,false));
      }
    }
    return out;
  }
  static vector<t_plan> get_movdirs_bow(int dirs,int sim_limit,real mag,int dt=8){
    vector<t_plan> out;auto dmag=mag*dt/sim_limit;
    real inv_dirs=1.0/dirs;
    for(int dir=0;dir<dirs;dir++)for(int sign=-1;sign<=+1;sign++)if(sign)
    {
      auto pos=vec2d(0,0);
      auto&b=qap_add_back(out);
      for(int i=1;i<=sim_limit/dt;i++){
        pos+=Vec2dEx(Pi+(dir+(0.05*i*sign))*inv_dirs*Pi2,dmag*2);
        b.add(i*dt,t_move().set(pos,false,false));
      }
    }
    return out;
  }
  struct t_event_counter{
    bool enabled=false;
    int n=0;
  };
  struct t_host;
  struct t_sim_v4_score_env{
    vec2d prev_ap;
    int pId=-1;
    t_plan plan;
    t_score out;
    vector<vector<t_player>>*plog=nullptr;
    //---
    t_host*phost=nullptr;
    t_foodmap*pfoodmap=nullptr;
    //---
    void bef(Mechanic&mech,int sim_limit)
    {
      mech.player_scores[pId]=0;
      out.bef=get_frags_info(mech,pId);
      prev_ap=out.bef.avg_pos;
      QapAssert(!plan.arr.empty());plan.arr.back().tick=sim_limit*2;
      if(pfoodmap)pfoodmap->sim_id++;
      if(phost&&phost->ec_burst.enabled){
        QapAssert(-1==mech.burst_hack.pId);
        QapAssert(!mech.burst_hack.n);
        mech.burst_hack.pId=pId;
      }
      if(phost&&phost->need_sim_without_burst){
        QapAssert(!mech.burst_hack.need_sim_without_burst);
        mech.burst_hack.need_sim_without_burst=true;
      }
    }
    void aft(Mechanic&mech)
    {
      out.score=mech.get_score_for(pId);
      out.aft=get_frags_info(mech,pId);
      out.dist=out.pos_diff();
      if(phost&&phost->ec_burst.enabled){
        QapAssert(mech.burst_hack.pId==pId);
        phost->ec_burst.n+=mech.burst_hack.n;
        mech.burst_hack.pId=-1;
        mech.burst_hack.n=0;
      }
      if(phost&&phost->need_sim_without_burst){
        QapAssert(mech.burst_hack.need_sim_without_burst);
      }
    }
    void apply_direct(Mechanic&mech,int i)
    {
      auto rec=plan.get_rec(i);QapAssert(rec.ok);auto&m=rec.pr.move;
      mech.apply_direct_for(pId,m);
      if(phost&&phost->ec_can_do_split.enabled)if(mech.can_player_do_split(pId)){
        //QapDebugMsg("need_track_can_do_split say :: pId = "+IToS(pId)+"\ni = "+IToS(i)+"\nmech.tick = "+IToS(mech.tick));
        phost->ec_can_do_split.n++;
      }
    }
    void iter_next(Mechanic&mech,int i)
    {
      auto rec=plan.get_rec(i);QapAssert(rec.ok);auto&m=rec.pr.move;
      out.score=mech.player_scores[pId];
      out.total_score+=out.score;
      if(plog)
      {
        auto&log=qap_add_back(*plog);
        t_world_parsed wp;
        load(wp,mech.player_array,CircleArray());
        log+=wp.parr;
      }
      auto info=get_frags_info(mech,pId);
      auto ap=info.avg_pos;
      out.total_dist+=ap.dist_to(prev_ap);
      out.mass=info.total_mass;
      out.total_mass+=info.total_mass;
      out.safety+=get_safety(mech,pId).total;
      out.total_safety+=out.safety;
      out.TFF=info.total_TFF;
      out.total_TFF+=out.TFF;
      prev_ap=ap;
      //
      if(pfoodmap&&pfoodmap->need_use_foodvalue){
        out.food+=pfoodmap->get_score(mech,pId);
        real tk=(sim_limit-i)/sim_limit;
        out.total_food+=out.food+out.food*tk*0.5;
        //out.total_food=0;
      }
    }
  };
  struct t_host{
    t_sim_env&env;
    vector<vector<t_player>>*plog=nullptr;
    t_foodmap*pfoodmap=nullptr;
    t_event_counter ec_can_do_split;
    t_event_counter ec_burst;
    bool need_sim_without_burst=false;
    t_host(t_sim_env&env,vector<vector<t_player>>*plog,t_foodmap*pfoodmap):env(env),plog(plog),pfoodmap(pfoodmap){}
    vector<vector<t_sim_v4_score_env>> arr;
    void next(){
      qap_add_back(arr);
    }
    t_sim_v4_score_env&add(){
      QapAssert(arr.size());
      auto&b=qap_add_back(arr.back());
      b.pId=env.pId;
      b.plan=env.plan;
      b.plog=plog;
      b.out.id=env.score.id;
      b.pfoodmap=pfoodmap;
      b.phost=this;
      return b;
    }
    t_score sum_all(){
      auto out=env.score;
      for(int i=0;i<arr.size();i++){
        auto&A=arr[i];
        QAP_FOREACH(A,out.add(ex.out));
      }
      return out.get_average();
    }
  };
  struct t_tracer_can_do_split{
    bool enabled=false;
    int n=0;
  };
  static void sim_v6_for_enemy(const t_conf&conf,const t_world_parsed&w,t_host&host,t_sim_v4_score_env&E,t_sim_env&env)
  {
    auto&mech=env.mech;
    to_mech(conf,w,mech);
    t_sim_v4_score_env&H=host.add();
    H.bef(mech,sim_limit);
    E.bef(mech,sim_limit);
    //run simulation
    for(int i=0;i<env.sim_limit;i++)
    {
      H.apply_direct(mech,i);
      E.apply_direct(mech,i);
      mech.tickEvent_v2(true);
      H.iter_next(mech,i);
      E.iter_next(mech,i);
    }
    H.aft(mech);
    E.aft(mech);
  }
  static void enemy_brain_v2(t_host&ehost,const t_conf&conf,const t_world_parsed&w,const t_movdirs&movdirs,t_host&host,t_plan_with_dbg&out,int enemy_pId)
  {
    vec2d avg_pos;int n=0;
    QAP_FOREACH(w.parr,if(ex.id!=enemy_pId)return;avg_pos+=ex.pos;n++;);avg_pos*=(1.0/n);
    if(!n){
      QapDebugMsg("hm...");
      return;
    }
    auto&envs=out.envs;
    vector<t_plan> curset;
    QAP_FOREACH(movdirs,qap_add_back(curset)=addbase(ex,avg_pos));
    envs.resize(curset.size());
    for(int i=0;i<curset.size();i++)
    {
      auto&env=envs[i];
      env.score.id=i;// score - need for probability
      env.pId=enemy_pId;
      env.sim_limit=sim_limit;
      env.plan=curset[i];
      //---
      t_sim_v4_score_env E;
      E.pId=env.pId;
      E.plan=env.plan;
      E.plog=nullptr;
      E.out.id=env.score.id;
      E.phost=&ehost;
      env.mech.score_hack.pId=env.pId;
      env.mech.score_hack.need_ignore_food=true;
      sim_v6_for_enemy(conf,w,host,E,env);
      env.score=E.out;
    }
    vector<t_score> tmp;tmp.resize(envs.size());
    QAP_FOREACH(envs,tmp[i]=ex.score);
    QAP_FOREACH(tmp,ex.total_safety=0);
    QAP_FOREACH(tmp,ex.total_dist=0);
    QAP_FOREACH(tmp,ex.total_TFF=0);
    qap_sort(tmp);
    update_vec_score_rank(tmp);
    QAP_FOREACH(tmp,envs[ex.id].score.set_rank(ex.rank,tmp.size()));
  }
  static void update_vec_score_rank(vector<t_score>&arr){
    int pos=-1;int n=0;if(arr.size())arr[0].rank=0;//bool dbg_even_happen=false;
    auto f=[&](){
      if(!n)return;
      if(1==n){arr[pos].rank=pos;pos=-1;n=0;return;}
      real avg_rank=0;
      for(int i=pos;i<pos+n;i++)avg_rank+=i;
      avg_rank/=n;
      for(int i=pos;i<pos+n;i++)arr[i].rank=avg_rank;
      #ifdef Adler
      //QapDebugMsg("got it\n n = "+IToS(n)+"\n pos = "+IToS(pos));dbg_even_happen=true;
      #endif
      pos=-1;n=0;
    };
    for(int i=1;i<arr.size();i++)
    {
      auto&a=arr[i-1];
      auto&b=arr[i-0];
      bool ok=a.almost_equal_to(b);
      if(ok)
      {
        if(!n){pos=i-1;n=1;}
        n++;
        int gg=1;
      }else{
        f();
        if(!n){pos=i;n=1;}
        f();
      }
      int gg=1;
    }
    f();
    int gg=1;
    #ifdef Adler
    //if(dbg_even_happen)
    //{
    //  vector<string> dbg;
    //  QAP_FOREACH(arr,qap_add_back(dbg)=jobj(
    //    ex.to_str(false)
    //    +","+jk("total_mass_and_total_score",(ex.total_mass_and_total_score()))
    //  ));
    //  string gg=join(dbg,"\n");
    //  file_put_contents("dbg.out.txt",gg);
    //  QapDebugMsg("now look at dbg.out.txt");
    //}
    #endif
  };
  static void sim_v4_full_for_part(t_host&ehost,const t_conf&conf,const t_world_parsed&w,t_host&host,const vector<int>&enemy_pIds,t_movdirs&enemy_movdirs)
  {
    vector<t_plan_with_dbg> pwds;pwds.resize(enemy_pIds.size());
    for(int i=0;i<enemy_pIds.size();i++){
      auto&epid=enemy_pIds[i];
      host.next();
      auto&pwd=pwds[i];
      enemy_brain_v2(ehost,conf,w,enemy_movdirs,host,pwd,epid);
      if(host.arr.back().size()!=pwd.envs.size()){
        QapDebugMsg("habs = "+IToS(host.arr.back().size())+"\npes = "+IToS(pwd.envs.size()));
        QapAssert(host.arr.back().size()==pwd.envs.size());
      }
      QAP_FOREACH(host.arr.back(),ex.out.probability=pwd.envs[i].score.probability);
      //host.arr[0].out.probability=pwd.envs[0].score.probability;
    }
    int gg=1;
  }
  int movdirs_for_enemy=12;
  t_movdirs part1,part2;
  void sim_v4_full_v2(t_sim_env&env,vector<vector<t_player>>*plog=nullptr)
  {
    auto enemy_pIds=world.get_enemy_pIds(env.pId);
    if(enemy_pIds.empty())
    {
      return sim_v5(env,plog);
    }
    env.mech.pfoodmap=&foodmap;
    if(part1.empty()||part2.empty()){
      part1=build_movdirs_for_enemy_v3(sim_limit,true,movdirs_for_enemy);
      part2=build_movdirs_for_enemy_v3(sim_limit,false,movdirs_for_enemy);
    }
    //
    t_host ehost(env,nullptr,nullptr);// WARN: don't use ehost.env or any other fields except t_event_counter
    ehost.ec_can_do_split.enabled=true;
    ehost.ec_burst.enabled=true;
    if(bool need_part1=true)
    {
      t_host host(env,plog,&foodmap);
      sim_v4_full_for_part(ehost,conf,world,host,enemy_pIds,part1);
      auto avg=host.sum_all();
      env.score.add(avg);
    }
    if(bool need_part1_b=ehost.ec_burst.n)
    {
      ehost.need_sim_without_burst=true;
      t_host host(env,plog,&foodmap);
      sim_v4_full_for_part(ehost,conf,world,host,enemy_pIds,part1);
      auto avg=host.sum_all();
      env.score.add(avg);
      if(bool more_probability_for_part1_b=true){
        avg.probability*=2.0;
      }
      ehost.need_sim_without_burst=false;
    }

    ehost.ec_can_do_split.enabled=false;
    ehost.ec_burst.n=0;
    if(bool need_part2=ehost.ec_can_do_split.n)
    {
      t_host host(env,plog,&foodmap);
      sim_v4_full_for_part(ehost,conf,world,host,enemy_pIds,part2);
      auto avg=host.sum_all();
      if(bool reduce_probability_for_part2=true){
        avg.probability*=0.5;
      }
      env.score.add(avg);
    }
    if(bool need_part2_b=ehost.ec_can_do_split.n&&ehost.ec_burst.n)
    {
      ehost.need_sim_without_burst=true;
      t_host host(env,plog,&foodmap);
      sim_v4_full_for_part(ehost,conf,world,host,enemy_pIds,part2);
      auto avg=host.sum_all();
      env.score.add(avg);
      ehost.need_sim_without_burst=false;
    }
    env.score=env.score.get_average();
    int gg=1;
  }
  void sim_v5_for_our(t_sim_v4_score_env&host_env,t_sim_env&env)
  {
    auto&mech=env.mech;
    to_mech(conf,world,mech);
    host_env.bef(mech,sim_limit);
    for(int i=0;i<env.sim_limit;i++)
    {
      host_env.apply_direct(mech,i);
      mech.tickEvent_v2(true);
      host_env.iter_next(mech,i);
    }
    host_env.aft(mech);
  }
  void sim_v5(t_sim_env&env,vector<vector<t_player>>*plog=nullptr)
  {
    t_sim_v4_score_env b;
    b.pId=env.pId;
    b.plan=env.plan;
    b.plog=plog;
    b.out.id=env.score.id;
    b.pfoodmap=&foodmap;
    env.mech.pfoodmap=&foodmap;
    sim_v5_for_our(b,env);
    env.score=b.out;
  }
  static void to_mech(const t_conf&conf,const t_world_parsed&w,Mechanic&out)
  {
    out.id_counter=7000;
    out.tick=w.tick;
    auto add_palyer=[&out](const t_player&p)
    {
      if(!p.me){
        auto tick=out.tick+1; // +1 because of tick_event impl
        auto watch_time=tick-p.found_tick;
        if(watch_time<=0)return;
      }
      auto&b=qap_add_back(out.player_array);b=new Player(p.id,p.pos.x,p.pos.y,p.r,p.m,p.fid);
      b->v=p.v;
      b->fuse_timer=p.TTF;
    };
    auto add_food=[&out,&conf](vec2d p)
    {
      qap_add_back(out.food_array)=new Food(out.id_counter++,p.x,p.y,FOOD_RADIUS,conf.FOOD_MASS);
    };
    auto add_virus=[&out,&conf](vec2d p,real m)
    {
      qap_add_back(out.virus_array)=new Virus(out.id_counter++,p.x,p.y,conf.VIRUS_RADIUS,m);
    };
    auto add_eject=[&out](vec2d p,int pId)
    {
      qap_add_back(out.eject_array)=new Ejection(out.id_counter++,p.x,p.y,EJECT_RADIUS,EJECT_MASS,pId);
    };
    //QAP_FOREACH(w.farr,auto dv=ex.pos-center;for(int i=0;i<4;i++){add_food(center+dv);dv=dv.Ort();});
    QAP_FOREACH(w.parr,add_palyer(ex));
    QAP_FOREACH(w.farr,add_food(ex.pos));
    QAP_FOREACH(w.varr,add_virus(ex.pos,ex.m));
    QAP_FOREACH(w.earr,add_eject(ex.pos,ex.pId));
    //QAP_FOREACH(w.parr,add_food(ex.pos));
  }
  static string jq(string value){static const string q="\"";return q+value+q;}
  static string jobj(string value){return "{"+value+"}";}
  static string jk(string key,string value){return jq(key)+":"+value;}
  static string jk(string key,double value){return jq(key)+":"+std::to_string(value);}
  static string dump(const vec2d&p){return jobj(jk("X",p.x)+","+jk("Y",p.y));}
  static string dump(const t_move&m,const string&raw_dbg){
    vector<string> out;
    qap_add_back(out)=jk("X",m.pos.x);
    qap_add_back(out)=jk("Y",m.pos.y);
    if(m.split)qap_add_back(out)=jk("Split","true");
    if(m.eject)qap_add_back(out)=jk("Eject","true");
    if(!raw_dbg.empty()&&jq("")!=raw_dbg)qap_add_back(out)=jk("Debug",raw_dbg);
    return jobj(join(out,","));
  }
  static string string2json(const string&s){
    Document d;
    d.SetString(s.c_str(),d.GetAllocator());
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
  }
  void init_main_app()
  {
    t_strategy&app=*this;
    if(bool like_old_top1=0)
    {
      app.init_movdirs(5);
      app.evolution_rot=true;
      app.evolution_split=true;
      app.use_center_plan=true;
      //app.server_side.TL*=0.1;
      return;
    }
    //app.evolution=true;
    if(bool based_on_strong_def=true)
    {
      //app.evolution_rot=true;
      app.evolution_split=true;
      app.smart_enemy=true;
      app.smart_v4=true;
      app.always_add_curplan=true;
      app.use_smart_v4_hardness_when_enemy=true;

      app.use_passive_movdirs_when_no_enemy=true;
      app.use_curplan_when_no_enemy=true;
      app.use_center_plan_when_no_enemy=true;

      app.init_movdirs(7);
      app.passive_movdirs=std::move(app.movdirs);
      app.init_movdirs(6);

      app.foodmap.need_use_inside_sim=false;
      app.foodmap.need_use_foodvalue=true;
      app.need_ignore_foodvalue_when_enemy=true;
      app.need_turn_off_foodmap_after_7k=true;

      app.movdirs_for_enemy=10;
      //app.server_side.clear();
      app.server_side.TL*=0.50*0.75;
    }
  }
};

struct BruteStrategy:public Strategy{
public:
  t_strategy app;
  int tick=0;
public:
  BruteStrategy(int id):Strategy(id),tick(0)
  {
    if(bool need_update_global_costants=true)
    {
      auto conf_str=file_get_contents("debug_constants.json");
      if(!conf_str.empty())
      {
        auto conf_doc=t_strategy::unsafe_get_doc(conf_str);
        t_conf&conf=app.conf;
        conf.load(conf_doc);
      }
    }
  }
  Direct tickEvent(const PlayerArray&farr,const CircleArray&carr)
  {
    app.world=t_world_parsed();
    load(app.world,farr,carr);
    app.world.tick=tick;
    auto out=app.qap_run_step();
    tick++;
    return out;
  }
};

#ifdef QAP_LITE_H
  class TGame:public TQapGameV2{
  public:
    real scale=1.1949681043624878;//1.17;
    vec2d world_wh=vec2d(990,990);
    vec2d cam_pos=world_wh*0.5;//vec2d(+1,-1)*512;
  public:
    #include "qaplite\QapKbOnDownDoInvFlag.inl"
  public:
    vector<std::unique_ptr<Strategy>> arr;
    Mechanic mechanic;
    bool need_init=true;
    std::unique_ptr<BruteStrategy> delayed_app;
    std::unique_ptr<BruteStrategy> up_app;
    std::unique_ptr<BruteStrategy> delayed_app2;
    std::unique_ptr<BruteStrategy> up_app2;
    std::unique_ptr<BruteStrategy> delayed_app3;
    std::unique_ptr<BruteStrategy> up_app3;
    std::unique_ptr<BruteStrategy> delayed_app4;
    std::unique_ptr<BruteStrategy> up_app4;
    real ls=0;
    void init(){
      static bool once=false;if(once)return;once=true;
      auto&c=Constants::instance();
      delayed_app.reset(new BruteStrategy(-1));
      world_wh=vec2d(c.GAME_WIDTH,c.GAME_HEIGHT);
      cam_pos=world_wh*0.5;
    }
  public:
    static void inv(volatile bool&flag){flag=!flag;}

    vec2d s2w(const vec2d&pos)
    {
      bool offcentric=false; auto cam_dir=vec2d(1,0);
      return t_offcentric_scope::screen_to_world(viewport,pos,cam_pos,cam_dir,scale,offcentric);
    }
    vec2d w2s(const vec2d&pos)
    {
      bool offcentric=false; auto cam_dir=vec2d(1,0);
      return t_offcentric_scope::make_xf(viewport,cam_pos,cam_dir,scale,offcentric)*pos;
    }
    int frame_id=0;int d_frame=1;
    void DoMove()
    {
      QAP_FOREACH(kb_flags,if(kb.OnDown(ex.key))inv(ex.flag));
      if(kb.Down[VK_ESCAPE])win.Close();
      if(kb.Down[VK_ESCAPE]&&kb.Down[VK_SHIFT]){TerminateProcess(GetCurrentProcess(),0);}
      if(kb.Down(VK_ADD))scale*=1.01;
      if(kb.Down(VK_SUBTRACT))scale/=1.01;
      if(kb.Down(VK_DIVIDE)){scale=1.19;cam_pos=world_wh*0.5;}
      if(kb.OnDown(VK_MULTIPLY))scale/=0.5;
      {cam_pos+=kb.get_dir_from_wasd_and_arrows()*(10.0/scale);}
      auto zK=1.5;
      if(zDelta>0){auto wp=s2w(mpos);scale*=zK;cam_pos+=(w2s(wp)-mpos)*(1.0/scale);}
      if(zDelta<0){auto wp=s2w(mpos);scale/=zK;cam_pos+=(w2s(wp)-mpos)*(1.0/scale);}
      //
      if(up_app)
      {
        auto*app=&up_app.get()->app;
        if(kb.OnDown(VK_SPACE))inv(app->pause);
        {cam_pos+=kb.get_dir_from_wasd_and_arrows()*(10.0/scale);}
        if(kb.OnDown(VK_HOME))frame_id=0;
        if(kb.OnDown(VK_END))frame_id=std::max<int>(0,int(app->replay.size())-1);
        if(kb.OnDown(VK_UP)){d_frame++;d_frame=Clamp<int>(d_frame,-1,+1);}
        if(kb.OnDown(VK_DOWN)){d_frame--;d_frame=Clamp<int>(d_frame,-1,+1);}
        for(int i=0;i<=9;i++)if(kb.OnDown('0'+i))d_frame=Sign(d_frame)*i;
        if(kb.Down(mbRight)){frame_id=app->replay.size()*(mpos.x/viewport.size.x+0.5);}
        frame_id+=d_frame;
        frame_id=app->replay.empty()?0:Clamp<int>(frame_id,0,int(app->replay.size())-1);
        if(kb.OnDown(VK_RIGHT)){
          if(frame_id<0)frame_id=int(app->replay.size())-1;
          frame_id++;
          if(frame_id>=app->replay.size()){
            frame_id=-1;
            app->lock_step=true;
          }
          //app->pause=true;
        }
        if(kb.OnDown(VK_LEFT)){
          if(frame_id<0)frame_id=int(app->replay.size())-1;
          frame_id--;
          if(frame_id<0)frame_id=0;
          //app->pause=true;
        }
        static vec2d drag_wp;
        if(kb.OnDown(mbLeft)){drag_wp=s2w(mpos);}
        if(kb.Down(mbLeft)){cam_pos+=-s2w(mpos)+drag_wp;}
      }
      //mechanic
      static auto seed="SEED100_eject 2018.04.20 23:38"; // <-------------------------------------------------------------------------------- SEED
      if(need_init)
      {
        need_init=false;
        delayed_app2.reset(new BruteStrategy(-1));delayed_app2->app.init_main_app();
        delayed_app3.reset(new BruteStrategy(-1));delayed_app3->app.init_main_app();
        delayed_app4.reset(new BruteStrategy(-1));delayed_app4->app.init_main_app();
        auto&app=delayed_app->app;
        app.init_main_app();
        mechanic.init_objects(seed,[&](Player*player)->Strategy*{
          int pId=player->getId();
          auto&b=qap_add_back(arr);
          if(!up_app){up_app=std::move(delayed_app);up_app->id=pId;return up_app.get();}
          //if(!up_app2){up_app2=std::move(delayed_app2);up_app2->id=pId;return up_app2.get();}
          //if(!up_app3){up_app3=std::move(delayed_app3);up_app3->id=pId;return up_app3.get();}
          //if(!up_app4){up_app4=std::move(delayed_app4);up_app4->id=pId;return up_app4.get();}
          //if(2==arr.size()){b.reset(new BruteStrategy(pId));return b.get();}
          //if(3==arr.size()){b.reset(new BruteStrategy(pId));return b.get();}
          b.reset(new Strategy(pId));
          return b.get();
        });
        //mechanic.player_array[0]->x=990/2;
        //mechanic.player_array[0]->y=990/2;
        //mechanic.player_array.push_back(new Player(90,990/2+12+12+12,990/2,PLAYER_RADIUS,PLAYER_MASS*4,3));.
        if(need_restore_mech_from_file)
        {
          mechanic.player_array.clear();
          mechanic.food_array.clear();
          mechanic.eject_array.clear();
          mechanic.virus_array.clear();
        
          app.go_dump_foodmap_and_world(5441,false);up_app->tick=app.world.tick;
          t_strategy::to_mech(app.conf,app.orig_world,mechanic);app.pause=true;
        }
      }
      int gg=1;
      static int speed=1;
      if(up_app)
      {
        if(!up_app->app.pause||up_app->app.lock_step)
        for(int i=0;i<speed;i++)
        {
          QapAssert(!mechanic.player_array.empty());

          if(need_restore_mech_from_file)
          {
            mechanic.player_array.clear();
            mechanic.food_array.clear();
        
            up_app->app.go_dump_foodmap_and_world(up_app->app.world.tick+1,false);up_app->tick=up_app->app.world.tick;
            t_strategy::to_mech(up_app->app.conf,up_app->app.orig_world,mechanic);up_app->app.pause=true;
          }
          mechanic.tickEvent_v2();

          if(0)if(mechanic.tick>=5440){
            speed=1;
            if(auto*p=up_app.get()){
              TDataIO IO;
              p->app.go_dump_foodmap_and_world(p->app.world.tick,true);
              p->app.pause=true;
            }
          }
          if(up_app->app.lock_step)break;
        }
        up_app->app.lock_step=false;
      }
      for(int i=0;i<=9;i++)if(kb.OnDown('0'+i))speed=i;
    }
    bool need_restore_mech_from_file=false;

    void clear(){D9Dev.Clear3D(1?0xffc8c8c8:0xff000000);}
    
    #define DECLARE(FIELD)auto&FIELD=qap_check_id(app->replay,frame_id)?app->replay[frame_id].FIELD:get_w_empty().FIELD;
    void DoDraw()
    {
      D9Dev.Set2D();
    
      D9Dev.pDev->SetRenderState(D3DRS_LIGHTING,false);
      D9Dev.pDev->SetRenderState(D3DRS_ZENABLE,false);
      D9Dev.pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
    
      qDev.BindTex(0,nullptr);
      QapDev::BatchScope Scope(qDev);
      {
        t_offcentric_scope scope(qDev,/*obj.pos+*/cam_pos,vec2d(1,0),scale,false);
        render();
      }
      //
      consize=win.GetClientSize();
      int y=0;int dy=16;
      auto add=[&](const string&text){
        y-=dy;
        auto backup_color=qDev.color;
        qDev.color=0xFF000000;
        qap_text::draw(qDev,viewport.get_vertex_by_dir(vec2d(-1,1))+vec2d(16+1,y-1),text,16);
        qDev.color=backup_color;
        qap_text::draw(qDev,viewport.get_vertex_by_dir(vec2d(-1,1))+vec2d(16,y),text,16);
      };
      qDev.color=0xff004000;
      add("mechanic.tick = "+IToS(mechanic.tick));
      QAP_FOREACH(mechanic.strategy_array,qDev.color=player2color(ex->id);add("player_scores["+IToS(ex->id)+"] = "+IToS(mechanic.player_scores[ex->id])));
      qDev.color=0xff004000;
      add("frame_id = "+IToS(frame_id));
      add("kb.LastKey = "+IToS(kb.LastKey));
      add("mpos = "+t_strategy::dump(s2w(mpos)));
      add("--- up_app ---");
      if(up_app)
      {
        auto&app=up_app.get()->app;
        auto&ass=app.server_side;
        if(ass.arr.size())
        {
          auto&b=ass.arr.back();
          add("dirs = "+IToS(b.dirs));
          add("ms/dir = "+std::to_string(b.ms_per_dir));
          add("ms = "+std::to_string(b.ms_per_dir*b.dirs));
          add("players = "+std::to_string(b.ppd));
          if(app.prev_cmd){
            app.prev_cmd->score.foreach([&](const string&name,double&value){add(name+"="+std::to_string(value));});
          }
        }
        if(QapKbOnDownDoInvFlag("need_show_dbg",'G',false)){
          auto arr=split(app.prev_cmd->dbg,":@@@:");
          QAP_FOREACH(arr,add("dbg["+IToS(i)+"] = "+ex));
        }
      }
      DrawDownedKeys();
    }
    void render()
    {
      ls=1/scale;
      //QapDev::BatchScope scope(qDev);
      qDev.color=0xff000000;
      qDev.DrawRectAsQuad(world_wh*0.5,world_wh,2);
      qDev.color=0xff000000;
      if(auto tilda=VK_OEM_3)if(kb.OnDown(tilda))
      {
        volatile int gg=1;
      }
      if(bool need_foodmap_draw_debug=false)if(kb.Down(mbRight))
      {
        auto mp=s2w(mpos);auto r=100;
        up_app->app.foodmap.draw(mp,r,up_app->tick,false);
        qDev.color=0xFF000000;
        qDev.DrawCircle(mp,r,0,1,128);
      }
      if(QapKbOnDownDoInvFlag("need_draw_foodmap",'M',true))if(up_app)
      {
        auto&app=up_app.get()->app;
        auto&arr=app.foodmap.map.mem;
        auto wh=app.foodmap.map.get_wh();
        auto cs=app.foodmap.half_cell_size*2;
        for(int i=0;i<arr.size();i++){
          auto&ex=arr[i];
          auto t=ex.get_beg_food_value(app.world.tick,app.foodmap.food_spawn_speed);//*ex.beg_food;//real(app.world.tick-ex.beg_t)/(1024*4);
          t=std::min<real>(t,1.0);
          qDev.color=QapColor::Mix(0x80000000,0x80ffffff,1.0-t);
          qDev.DrawQuad(vec2d(i%int(wh.x),i/int(wh.y)).Mul(cs)+app.foodmap.half_cell_size,cs);
        }
      }
      if(QapKbOnDownDoInvFlag("need_draw_future",'F',true))if(up_app)
      {
        auto&app=up_app.get()->app;
        if(auto*pcmd=app.prev_cmd.get())
        {
          auto&s=pcmd->score;
          //qDev.color=0x8000ff00;
          //qDev.DrawLine(s.bef.avg_pos,s.aft.avg_pos,ls*4);
          if(QapKbOnDownDoInvFlag("need_draw_plan_points",'P',false))
          {
            qDev.color=0xFF0000FF;
            auto&arr=pcmd->plan.arr;
            auto pp=s.bef.avg_pos;
            for(int i=0;i<arr.size();i++)
            {
              auto&ex=arr[i];
              auto cur=Direct::limit(ex.move.pos);
              qDev.color=0x8000AA20;
              qDev.DrawQuad(cur,vec2d(1,1)*ls*16,PiD4);
              qDev.color=0x20808800;
              qDev.DrawLine(pp,cur,ls*4);
              pp=cur;
            }
          }
          if(QapKbOnDownDoInvFlag("need_draw_track_from_players_log",'T',true))
          {
            auto&arr=pcmd->players_logs;
            for(int i=0;i<arr.size();i++)
            {
              auto&parr=arr[i];
              for(int i=0;i<parr.size();i++)
              {
                auto&ex=parr[i];
                qDev.color=QapColor::HalfMix(0xFF000000,player2color(ex.id));
                qDev.color.a=0x20;
                qDev.DrawQuad(ex.pos,vec2d(1,1)*ls*8);
              }
            }
          }
          if(QapKbOnDownDoInvFlag("need_draw_all_future_lines",'L',false))
          {
            auto&arr=pcmd->envs;
            for(int i=0;i<arr.size();i++){
              auto&ex=arr[i];
              auto&s=ex.score;
              qDev.color=0x80000000;
              qDev.DrawLine(s.bef.avg_pos,s.aft.avg_pos,ls);
            }
          }
          if(qap_check_id(pcmd->envs,pcmd->score.id))
          {
            auto arr=pcmd->envs[pcmd->score.id].mech.player_array;
            for(int i=0;i<arr.size();i++){
              auto&ex=arr[i];
              draw(ex,true);
            }
          }
        }
      }
      qDev.color=0xff000000;
      QAP_FOREACH(mechanic.eject_array,
        qDev.DrawSolidCircle(get_pos(*ex),ex->radius,8,0);
      );
      qDev.color=0xff000000;
      QAP_FOREACH(mechanic.food_array,
        qDev.DrawSolidCircle(get_pos(*ex),ex->radius,5,0);
      );
      qDev.color=0xff005000;
      QAP_FOREACH(mechanic.virus_array,
        qDev.DrawSolidCircle(get_pos(*ex),ex->radius,16,0);
      );
      if(QapKbOnDownDoInvFlag("need_draw_vision",'V',true))
      {
        qDev.color=0xff808080;
        QAP_FOREACH(mechanic.player_array,draw(ex,false,true));
      }
      qDev.color=0xff808080;
      QAP_FOREACH(mechanic.player_array,draw(ex,false,false));
    }
    void draw(Player*ex,bool future=false,bool background=false){
      if(background)
      {
        if(future)return;
        qDev.color=0x10000000;
        qDev.DrawSolidCircle(ex->get_vis_center(),ex->vision_radius,32,0);
        
        //qDev.color=0xFF0000FF;
        //qDev.DrawCircle(ex->get_vis_center(),Player::get_vision_radius(ex->radius,mechanic.get_fragments_cnt(ex->id)),0,ls,32);
        return;
      }
      qDev.color=player2color(ex->id);if(future)qDev.color.a=0x30;
      qDev.DrawSolidCircle(get_pos(*ex),ex->radius,16,0);
      qDev.color=0xff000000;if(future)qDev.color.a=0x30;
      qDev.DrawCircle(get_pos(*ex),ex->radius,0,ls,32);
    }
    QapColor player2color(int player_id){
      QapColor out=0xff000000;
      auto f=[&](int id,QapColor color){if(id==player_id%4)out=color;};
      f(0,0xABFF0000);
      f(1,0xABFFAA00);
      f(2,0xABAAFF00);
      f(3,0xAB0000FF);
      return out;
    }
    template<class TYPE>
    vec2d get_pos(const TYPE&obj){return vec2d(obj.x,obj.y);}
    #undef DECLARE
    static string BToS(bool b){return b?"true":"false";}
  };
  void win_main()
  {
    static GlobalEnv global_env(GetModuleHandle(NULL),0,"",SW_SHOW);
    auto on_heap=std::make_unique<TGame>();
    TGame&builder=*on_heap;
    builder.init();
    builder.DoNice();
  }
#endif

int main()
{
  //perror("build at 2018.04.29 20:22:19 commit 2dac6bb4e96029384c6b608910aaa283de5d8826");
  #if(defined(Adler)&&!defined(LOGREADMODE))
    win_main();return 0;
  #endif
  t_strategy s;
  s.init_main_app();
  s.init();
  s.run();
}
