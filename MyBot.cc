#include <bits/stdc++.h>
#include "PlanetWars.h"

// Global Vars
int turn_no = 0;
int future_turns = 50;
int self_owner;
int enemy_owner;
int frontier_size = 2;
std::vector<Planet> enemy_planets;
std::vector<Planet> my_planets;
std::vector<double> dist_my_planets;
std::vector<double> dist_enemy_planets;
std::vector<int> closest_my_planets;
std::vector<double> base_strength;
std::map<double, int> src_distances;
std::set<int> src_sorted_ids;
std::vector<double> dest_Score;
std::map<double, int> frontier_value;
std::set<int> frontier;
bool once = true;
int destID = 0;
int sniper_limit = 7;
const PlanetWars* gpw;

// Future Class and Init
class PlanetFuture{
private:
  int num_ships_;
  int owner_;

public:
  PlanetFuture(){
  }

  PlanetFuture(const PlanetFuture &p) {
    num_ships_ = p.GetNumShips();
    owner_ = p.GetOwner();
  }

  void operator=(const PlanetFuture &p) {
    num_ships_ = p.GetNumShips();
    owner_ = p.GetOwner();
  }

  int GetOwner() const {
    return owner_;
  }

  int GetNumShips() const {
    return num_ships_;
  }

  void SetNumShips(int num_ships){
    num_ships_ = num_ships;
  }

  void SetOwner(int owner){
    owner_ = owner;
  }

  void AddShips(int ships){
    num_ships_ += ships;
  }

};

class DummyFleet{
public:
  int src, dest, numShips, owner, turns;
  DummyFleet(){

  }

  DummyFleet(int s, int d, int n, int o, int t){
    src = s; dest = d; numShips = n; owner = o; turns = t;
  }
};

std::vector<DummyFleet> dummy_fleets;
PlanetFuture future_planets [100][100];

void getFuture(){
  std::vector<Fleet> fleets = gpw->Fleets();
  for(int i=0; i< gpw->Planets().size(); i++){
    future_planets[i][0].SetNumShips(gpw->GetPlanet(i).NumShips());
    future_planets[i][0].SetOwner(gpw->GetPlanet(i).Owner()); 
  }
  for(int t=1; t<future_turns; t++){
    for(int p=0; p< gpw->Planets().size(); p++){
      future_planets[p][t] = future_planets[p][t-1];
      if(future_planets[p][t].GetOwner() !=0)
        future_planets[p][t].AddShips(gpw->GetPlanet(p).GrowthRate());
    }
    for(int i=0; i<fleets.size(); i++){ 
      if(fleets[i].TurnsRemaining() == t){
        if(fleets[i].Owner()==future_planets[fleets[i].DestinationPlanet()][t].GetOwner())
          future_planets[fleets[i].DestinationPlanet()][t].AddShips(fleets[i].NumShips());
        else{
          future_planets[fleets[i].DestinationPlanet()][t].AddShips(-1*fleets[i].NumShips());
          if(future_planets[fleets[i].DestinationPlanet()][t].GetOwner() == 0){ 
            if(future_planets[fleets[i].DestinationPlanet()][t].GetNumShips() < 0){
              future_planets[fleets[i].DestinationPlanet()][t].SetNumShips(-1*future_planets[fleets[i].DestinationPlanet()][t].GetNumShips());
              future_planets[fleets[i].DestinationPlanet()][t].SetOwner(fleets[i].Owner());      
            }
          }
        }      
      }
    }
    for(int i = 0; i < gpw->Planets().size(); i++){
      if(future_planets[i][t].GetNumShips() < 0){
        future_planets[i][t].SetNumShips(-1*future_planets[i][t].GetNumShips());
        future_planets[i][t].SetOwner(3 - future_planets[i][t].GetOwner()); 
      }
    }
  }
}

// Update Dest
void updateFuture(int src, int dest){
  
  std::vector<Fleet> fleets = gpw->Fleets();
  std::vector<Fleet> src_fleets, dest_fleets;

  future_planets[src][0].SetNumShips(gpw->GetPlanet(src).NumShips());
  future_planets[dest][0].SetNumShips(gpw->GetPlanet(dest).NumShips());

  for(int i = 0; i < dummy_fleets.size(); i++){
      if(dummy_fleets[i].src == src){
        future_planets[src][0].AddShips(-dummy_fleets[i].numShips);
      }
      if(dummy_fleets[i].src == dest){
        future_planets[dest][0].AddShips(-dummy_fleets[i].numShips);
      }
  }
  for(int i = 0; i < fleets.size(); i++){
    if(fleets[i].DestinationPlanet() == src)
      src_fleets.push_back(fleets[i]);
    else if(fleets[i].DestinationPlanet() == dest)
      dest_fleets.push_back(fleets[i]);
  }

  for(int t=1; t<future_turns; t++){
    future_planets[src][t] = future_planets[src][t-1];
    if(gpw->GetPlanet(src).Owner()!=0)
      future_planets[src][t].AddShips(gpw->GetPlanet(src).GrowthRate());
    for(int i=0; i<src_fleets.size(); i++){ 
      if(src_fleets[i].TurnsRemaining() == t){
        if(src_fleets[i].Owner()==future_planets[src][t].GetOwner())
          future_planets[src][t].AddShips(src_fleets[i].NumShips());
        else{
          future_planets[src][t].AddShips(-1*src_fleets[i].NumShips());
        }      
      }
    }
    for(int i = 0; i < dummy_fleets.size(); i++){
      if(dummy_fleets[i].dest == src && dummy_fleets[i].turns == t){
        if(dummy_fleets[i].owner == future_planets[src][t].GetOwner())
          future_planets[src][t].AddShips(dummy_fleets[i].numShips);
        else future_planets[src][t].AddShips(-dummy_fleets[i].numShips);
      }
    }
    if(future_planets[src][t].GetNumShips() < 0){
      future_planets[src][t].SetNumShips(-1*future_planets[src][t].GetNumShips());
      future_planets[src][t].SetOwner(3 - future_planets[src][t].GetOwner()); 
    }
  }
  
  if(dest > 0){
    for(int t=1; t<future_turns; t++){
      future_planets[dest][t] = future_planets[dest][t-1];
      if(gpw->GetPlanet(dest).Owner()!=0)
        future_planets[dest][t].AddShips(gpw->GetPlanet(dest).GrowthRate());
      for(int i=0; i<dest_fleets.size(); i++){ 
        if(dest_fleets[i].TurnsRemaining() == t){
          if(dest_fleets[i].Owner()==future_planets[dest_fleets[i].DestinationPlanet()][t].GetOwner())
            future_planets[dest_fleets[i].DestinationPlanet()][t].AddShips(dest_fleets[i].NumShips());
          else{
            future_planets[dest_fleets[i].DestinationPlanet()][t].AddShips(-1*dest_fleets[i].NumShips());
            if(future_planets[dest_fleets[i].DestinationPlanet()][t].GetNumShips() < 0 && future_planets[dest][t].GetOwner() == 0){
              future_planets[dest_fleets[i].DestinationPlanet()][t].SetNumShips(-1*future_planets[dest_fleets[i].DestinationPlanet()][t].GetNumShips());
              future_planets[dest_fleets[i].DestinationPlanet()][t].SetOwner(dest_fleets[i].Owner()); 
            }
          }      
        }
      }
      for(int i = 0; i < dummy_fleets.size(); i++){
        if(dummy_fleets[i].dest == dest && dummy_fleets[i].turns == t){
          if(dummy_fleets[i].owner == future_planets[dest][t].GetOwner())
           future_planets[dest][t].AddShips(dummy_fleets[i].numShips);
          else future_planets[dest][t].AddShips(-dummy_fleets[i].numShips);
        }
      }
      if(future_planets[dest][t].GetNumShips() < 0){
      future_planets[dest][t].SetNumShips(-1*future_planets[dest][t].GetNumShips());
      future_planets[dest][t].SetOwner(3 - future_planets[dest][t].GetOwner()); 
    }
    }
  }
}

// Calc Distance for all planets from all of our planets.
void calcDistance(){
  int size = (gpw->Planets()).size();
  dist_my_planets.clear();
  dist_enemy_planets.clear();
  closest_my_planets.clear();
  base_strength.clear();
  dist_my_planets.resize(size);
  dist_enemy_planets.resize(size);
  closest_my_planets.resize(size);
  base_strength.resize(size);
  std::vector<Planet> enemy_planets =  gpw->EnemyPlanets();
  for(int p = 0; p < size; p++){
    double dist = 0;
    int min_dist = 9999999;
    double base = 0;
    double enemy_dist = 0;
    for (int i = 0; i < my_planets.size(); i++){
      if(my_planets[i].PlanetID() == p) continue;
      dist += gpw->Distance(my_planets[i].PlanetID(),p)*gpw->Distance(my_planets[i].PlanetID(),p);
      min_dist = std::min(min_dist,gpw->Distance(my_planets[i].PlanetID(),p));
    }
    for(int i = 0; i < enemy_planets.size(); i++){
      enemy_dist += gpw->Distance(enemy_planets[i].PlanetID(),p)*gpw->Distance(enemy_planets[i].PlanetID(),p);
    }
    // Base strength with present
    for (int i = 0; i < size; i++){
      double dist = gpw->Distance(i,p);
      if(i==p){
        dist = 1;
      } 
      else if(gpw->GetPlanet(i).Owner() == self_owner){
        base += ((gpw->GetPlanet(i).NumShips())/ dist);
      }
      else if(gpw->GetPlanet(i).Owner() == enemy_owner){
        base -= ((gpw->GetPlanet(i).NumShips())/ dist); 
      }
    }
    
    std::vector<Fleet> fleets = gpw->Fleets();
    for(int i = 0; i < fleets.size(); i++ ){
      double fleet_dist_imputed = (1.0*(gpw->Distance(p,fleets[i].DestinationPlanet())*(fleets[i].TotalTripLength() - fleets[i].TurnsRemaining()) + gpw->Distance(p,fleets[i].SourcePlanet())*fleets[i].TurnsRemaining()))/fleets[i].TotalTripLength();
      if(fleets[i].Owner() == self_owner){
        base += fleets[i].NumShips()/fleet_dist_imputed;
      }
      else{
        base -= fleets[i].NumShips()/fleet_dist_imputed;
      }
    } 
    
    closest_my_planets[p] = min_dist;
    dist_my_planets[p] = std::sqrt(dist);
    base = ((1.0 * base)/(std::abs(base)))*std::sqrt(std::abs(base));
    base_strength[p] = base;
    dist_enemy_planets[p] = std::sqrt(enemy_dist);
    }
}

void getFrontier(){
  frontier.clear();
  frontier_value.clear();
  std::map<int,int> enemy_map;
  std::vector<Fleet> enemy_fleet1 = gpw->EnemyFleets();
  for(int i = 0; i < enemy_planets.size(); i++){
    int val = enemy_planets[i].NumShips();
    for (int j = 0; j < enemy_fleet1.size(); ++j)
    {
      if(enemy_fleet1[j].SourcePlanet() == enemy_planets[i].PlanetID()){
        val+= enemy_fleet1[j].NumShips();
      }
    }
    enemy_map.insert(std::make_pair(val ,enemy_planets[i].PlanetID()));
  }
  
  std:: map<int, int> :: iterator it = enemy_map.end();
  it--;
  std:: map<int, int> close_planets;
  for(int i = 0; i < my_planets.size(); i++) close_planets.insert(std::make_pair(gpw->Distance(it->second,my_planets[i].PlanetID()),my_planets[i].PlanetID()));
  frontier.insert(close_planets.begin() -> second);
  it--;
  close_planets.clear();
  for(int i = 0; i < my_planets.size(); i++) close_planets.insert(std::make_pair(gpw->Distance(it->second,my_planets[i].PlanetID()),my_planets[i].PlanetID()));
  frontier.insert(close_planets.begin() -> second);
}

int getSurplus(int id){
  int surp = 99999;
  
  // //Don't attack if enemy is too close.
  // int dist = 99999;
  // int enemy_ships = 0;
  // for(int i = 0; i < enemy_planets.size(); i++){
  //   if(dist > gpw->Distance(enemy_planets[i].PlanetID(),id)){
  //     dist = gpw->Distance(enemy_planets[i].PlanetID(),id);
  //     enemy_ships = enemy_planets[i].NumShips();
  //   }
  // }
  // if(gpw->GetPlanet(id).NumShips() - dist*gpw->GetPlanet(id).GrowthRate() < enemy_ships) return 0;
  
  // Calculate surplus
  
  for(int i = 0; i < future_turns-1; ++i){
    surp = std::min(surp,future_planets[id][i].GetNumShips() - gpw->GetPlanet(id).GrowthRate()*i);  
  }
  return surp;
}

double calHeuristic(const Planet& i){
  int id = i.PlanetID();
  double dist_frontier = 0;
  int closest_frontier = 99999;
  for(std::set<int>::iterator it = frontier.begin(); it != frontier.end(); it++){
    closest_frontier = std::min(gpw->Distance(id,*it), closest_frontier);
  }
  int strength = 9999999;
  int a1 = future_planets[id][closest_frontier].GetNumShips();
  int a2 = (future_planets[id][future_turns-1].GetNumShips() - (future_turns - closest_frontier)*i.GrowthRate());
  strength = std::max(a1,a2);
  double h = - std::sqrt(strength) + i.GrowthRate() + base_strength[id];
  return h;
}

struct not_my_planets_for_attack{
  bool operator() (Planet a, Planet b){
    double ha = calHeuristic(a);
    double hb = calHeuristic(b);
    return ha > hb;
  }
};


struct my_planets_for_attack{
  bool operator() (Planet a, Planet b){
    int id_a = a.PlanetID();
    int id_b = b.PlanetID();
    int strength_a = 9999999;
    int strength_b = 9999999;
    double ha = a.GrowthRate() + 0.1 * base_strength[a.PlanetID()];
    double hb = b.GrowthRate() + 0.1 * base_strength[b.PlanetID()];
    return ha > hb;
  }
};

double getScore(Planet& src, const Planet& dest){
  double dist = gpw->Distance(src.PlanetID(), dest.PlanetID());
  double growth = dest.GrowthRate();
  double score = (1.0 +growth) / ((1.0 + dest.NumShips() + dist));
  return score;
}

bool canAttack(int src, int dec){
  for (int i = 0; i < future_turns-1; ++i)
  {
    if(future_planets[src][i].GetNumShips() - dec < 0)
      return false; 
  }
  return true;
}

int find_shift(int dest){
  for (int i = 0; i < future_turns; ++i)
  {
    if(future_planets[dest][i].GetOwner() == enemy_owner){
      return i;     
    }
  }
  return -1;
}

bool possibleSniping(int dest, int& src, int& strength){
  int current_owner = future_planets[dest][0].GetOwner();
  int final_owner = future_planets[dest][future_turns-1].GetOwner();
  if(final_owner == enemy_owner){
    int change = find_shift(dest);
    src_distances.clear();
    for(int i=0; i<my_planets.size(); i++){
    	src_distances.insert(std::make_pair(gpw->Distance(my_planets[i].PlanetID(),dest),my_planets[i].PlanetID()));
	}
	std::map<double,int>::iterator it = src_distances.begin();
    for (it; it != src_distances.end() ; it++)
    {	
    	if(it->first > sniper_limit) break;
    	src = it->second;
    	strength = future_planets[dest][gpw->Distance(it->second,dest)].GetNumShips() + 1;
    	if(future_planets[it->second][0].GetNumShips() > strength && canAttack(it->second,strength)){
    		return true;
	    }
    }
  }
  return false;
}


void DoTurn(const PlanetWars& pw) {
  dummy_fleets.clear();
  
  gpw = &pw;
  
  dest_Score.clear();
  std::vector<Planet> planets = gpw->Planets();
  std::vector<Fleet> fleet = gpw->MyFleets();
  std::vector<Planet> neutral_planets = gpw->NeutralPlanets();
  enemy_planets = gpw->EnemyPlanets();
  my_planets = gpw->MyPlanets();
  dest_Score.resize(planets.size());
  
  if(once){
    self_owner = my_planets[0].Owner();
    enemy_owner = 3 - self_owner;
    once = false;
  }

  // no planet to attack
  if(my_planets.size() == 0) return;
  //game has been won
  if(enemy_planets.size() <= 0) return;
  // get the future, and initialize the imp vectors
  getFuture();
  calcDistance();
  getFrontier();
  
  std::vector<Planet> not_my_planets_now = gpw->NotMyPlanets();
  
  std::vector<Planet> neutral; // to store the planets which are not in our control at the end of the future calculation
  std::vector<Planet> enemy; // to store the planets which are not in our control at the end of the future calculation
  std::vector<Planet> own; // to store my planets in danger
  
  int src, str;
  for (int i = 0; i < planets.size(); ++i)
  {
    int id = planets[i].PlanetID();
    bool myown = false;
    if(future_planets[id][future_turns-1].GetOwner() != self_owner){
      for (int j = 0; j < future_turns-1; ++j)
      {
        if (future_planets[id][j].GetOwner() == self_owner)
        {
          own.push_back(planets[i]);
          myown = true;
          break;
        }
      }
      
      if(myown == false){
        if(future_planets[id][future_turns-1].GetOwner() == enemy_owner){
          enemy.push_back(planets[i]);
        }
        else{
          neutral.push_back(planets[i]);
        }
      }
    }
  }

  //checking the various data
  for(int i = 0; i< planets.size(); i++){
    int b1 = future_planets[i][std::min(closest_my_planets[i],future_turns-1)].GetNumShips();
    int b2 = future_planets[i][future_turns-1].GetNumShips() - (future_turns - closest_my_planets[i])*gpw->GetPlanet(i).GrowthRate();
    int num_ships = gpw->GetPlanet(i).NumShips();
    int growth = gpw->GetPlanet(i).GrowthRate();
    int dist1 = dist_my_planets[i];
    int closest = closest_my_planets[i];
    int strength = std::max(b1, b2);
    double num = (growth*1.0 + 0.1*base_strength[i]);
    double denom = (1.0*dist1+strength/10.0);
    double rel = 0;
    if(future_planets[i][0].GetOwner() != self_owner){
   		rel = calHeuristic(planets[i]);
    }
    else{
      rel = growth + 0.1 * base_strength[i];
    }
    dest_Score[i] = rel;
  }
  
  sort(own.begin(), own.end(), my_planets_for_attack());
  sort(enemy.begin(), enemy.end(), not_my_planets_for_attack()); // because one was not working out
  sort(neutral.begin(), neutral.end(), not_my_planets_for_attack()); // because one was not working out
  
  for(int i = 0; i < own.size(); i++){
    src_distances.clear();
    int id_dest = own[i].PlanetID();
    for (int j = 0; j < my_planets.size(); ++j)
    {
      int h_distance = gpw->Distance(id_dest, my_planets[j].PlanetID());
      int strength = my_planets[j].NumShips();
      int gr = my_planets[j].GrowthRate();
      strength = std::max(1, strength); 
      double heuristic = h_distance/(1.0*strength);
      src_distances.insert(std::make_pair(heuristic, my_planets[j].PlanetID()));
    }

    std::map<double, int> :: iterator it = src_distances.begin();
    for (it; it != src_distances.end() ; it++)
    {
      if(id_dest != it->second){
        int dist = gpw->Distance(id_dest, it->second);
        if(dist == 0)
          continue;
        int maxMinShips;
        int change = find_shift(id_dest);
        if(dist > change){
          maxMinShips = future_planets[id_dest][dist].GetNumShips()+2;
        }
        else{
          maxMinShips = future_planets[id_dest][change].GetNumShips()+1;
        }
        maxMinShips = std::max(1, maxMinShips);
        if(future_planets[(*it).second][0].GetNumShips() > maxMinShips && canAttack((*it).second, maxMinShips)){
          gpw->IssueOrder((*it).second, own[i].PlanetID(), maxMinShips);
          dummy_fleets.push_back(DummyFleet((*it).second, own[i].PlanetID(), maxMinShips,gpw->GetPlanet(it->second).Owner() ,dist));
          updateFuture( (*it).second, own[i].PlanetID());
          break;
        }
      }  
    }
  }

  for(int i=0; i<neutral_planets.size(); i++){
    if(possibleSniping(neutral_planets[i].PlanetID(),src, str)){
      gpw->IssueOrder(src,neutral_planets[i].PlanetID(), str);
      dummy_fleets.push_back(DummyFleet(src, neutral_planets[i].PlanetID(), str, gpw->GetPlanet(src).Owner(), gpw->Distance(src, neutral_planets[i].PlanetID())));
      updateFuture(src, neutral_planets[i].PlanetID());
    }
  }
  
  for(int i = 0; i < enemy.size(); i++){
    
    int frontier_strength = 0;
    std::set<int>::iterator itf = frontier.begin();
    for(int j = 0; j < frontier.size(); j++){
      frontier_strength += future_planets[*itf][0].GetNumShips();
      itf++;
    }
    if(dest_Score[enemy[i].PlanetID()] > -0.2){
      if(frontier_strength <= 0.5*gpw->NumShips(self_owner) &&  dist_my_planets[enemy[i].PlanetID()] > dist_enemy_planets[enemy[i].PlanetID()]) continue;
      src_distances.clear();
      
      destID = enemy[i].PlanetID();
      for (int j = 0; j < my_planets.size(); ++j)
      {
        int h_distance = gpw->Distance(destID, my_planets[j].PlanetID());
        int strength = my_planets[j].NumShips();
        int gr = my_planets[j].GrowthRate();
        double heuristic = h_distance - 0.1*strength + 0.3*gr; 
        src_distances.insert(std::make_pair(heuristic, my_planets[j].PlanetID()));
      }

      std::map<double, int> :: iterator it = src_distances.begin();
      for (it; it != src_distances.end() ; it++)
      {
        if(it->second != destID){
          int dist = gpw->Distance(destID, it->second);
          int maxMinShips, id_dest = enemy[i].PlanetID();
          int a1 = future_planets[id_dest][dist].GetNumShips();
          int a2 = (future_planets[id_dest][future_turns-1].GetNumShips() - (future_turns - dist)*enemy[i].GrowthRate());
          maxMinShips = std::max(a1,a2) + 1 ;
          if(future_planets[(*it).second][0].GetNumShips() > maxMinShips && canAttack((*it).second, maxMinShips)){
            gpw->IssueOrder((*it).second, enemy[i].PlanetID(), maxMinShips);
            dummy_fleets.push_back(DummyFleet((*it).second, enemy[i].PlanetID(), maxMinShips, gpw->GetPlanet(it->second).Owner(),dist));
            updateFuture((*it).second, enemy[i].PlanetID());
            break;
          }
        }  
      }
    }
    
  }

  for(int i = 0; i < neutral.size(); i++){
    int frontier_strength = 0;
    std::set<int>::iterator itf = frontier.begin();
    for(int j = 0; j < frontier.size(); j++){
      frontier_strength += future_planets[*itf][0].GetNumShips();
      itf++;
    }
    if(frontier_strength <= 0.5*gpw->NumShips(self_owner) &&  dist_my_planets[neutral[i].PlanetID()] > dist_enemy_planets[neutral[i].PlanetID()]) continue;
    if(dest_Score[neutral[i].PlanetID()] > 0 ){
      src_distances.clear();
      
      destID = neutral[i].PlanetID();
      for (int j = 0; j < my_planets.size(); ++j)
      {
        int h_distance = gpw->Distance(destID, my_planets[j].PlanetID());
        int strength = my_planets[j].NumShips();
        int gr = my_planets[j].GrowthRate();
        double heuristic = h_distance - 0.1*strength + 0.3*gr; 
        src_distances.insert(std::make_pair(heuristic, my_planets[j].PlanetID()));
      }

      std::map<double, int> :: iterator it = src_distances.begin();
      for (it; it != src_distances.end() ; it++)
      {
        if(it->second != destID){
          int dist = gpw->Distance(destID, it->second);
          int maxMinShips, id_dest = neutral[i].PlanetID();
          int a1 = future_planets[id_dest][dist].GetNumShips();
          int a2 = (future_planets[id_dest][future_turns-1].GetNumShips() - (future_turns - dist)*neutral[i].GrowthRate());
          maxMinShips = std::max(a1,a2) + 1 ;
          if(future_planets[(*it).second][0].GetNumShips() > maxMinShips && canAttack((*it).second, maxMinShips) && neutral[i].GrowthRate() > 0){
            gpw->IssueOrder((*it).second, neutral[i].PlanetID(), maxMinShips);
            dummy_fleets.push_back(DummyFleet((*it).second, neutral[i].PlanetID(), maxMinShips,gpw->GetPlanet(it->second).Owner() ,dist));
            updateFuture((*it).second, neutral[i].PlanetID());
            break;
          }
        }  
      }
    }
    
  }

  // Frontier Strengthening
  if(my_planets.size() > frontier_size){
    getFrontier();
    std::set<int>::iterator itt = frontier.begin();
    for(int i = 0; i < frontier.size(); i++){
      itt++;
    }
    for(int i = 0; i < my_planets.size(); i++){
      if(frontier.find(my_planets[i].PlanetID()) == frontier.end()){
        int frontier_id = 99999;
        int frontier_dist = 9999;
        std::set<int>::iterator it = frontier.begin();
        //Send to the closest one
        for(int j = 0; j < frontier.size(); j++){
          int new_dist = gpw->Distance(*it, my_planets[i].PlanetID());
          // insert if don't want to send to frontier too close to planet
          if(frontier_dist > new_dist){
            frontier_dist = new_dist;
            frontier_id = *it;
          }
          it++;
        }
        if(find_shift(my_planets[i].PlanetID()) == -1 && getSurplus(my_planets[i].PlanetID()) > 0){
          gpw->IssueOrder(my_planets[i].PlanetID(), frontier_id, getSurplus(my_planets[i].PlanetID()));
          dummy_fleets.push_back(DummyFleet(my_planets[i].PlanetID(), frontier_id, getSurplus(my_planets[i].PlanetID()), my_planets[i].Owner() ,gpw->Distance(my_planets[i].PlanetID(),frontier_id)));
          updateFuture(my_planets[i].PlanetID(), frontier_id);  
        }
        
     
      }
    }
  }
  
}

// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
  std::string current_line;
  std::string map_data;
  while (true) {
    int c = std::cin.get();
    current_line += (char)c;
    if (c == '\n') {
      if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
        PlanetWars pw(map_data);
        map_data = "";
        DoTurn(pw);
	pw.FinishTurn();
      } else {
        map_data += current_line;
      }
      current_line = "";
    }
  }
  return 0;
}
