#include "Player.hh"
#include <set>
#include <map>

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Bob


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /**
   * Types and attributes for your player can be defined here.
   */
   enum Casella{
     ENEMIC,
     AMIC,
     MASCARA,
     NO
   };


   typedef vector<int> VI;
   typedef vector<VI> VVI;
   typedef vector<bool> VB;
   typedef vector<VB> VVB;
   typedef vector<CellType> VE;
   typedef vector<VE> VVE;
   typedef vector<Casella> VC;
   typedef vector<VC> VVC;

   /////////// Funcions ////////////////

   // Checks wheater a int is in the set
   bool in_set(int id, const set<int>& units)
   {
     return (units.find(id) != units.end());
   }

   // Returns all the data into the 'mapa'
   void obtain_all_data(VVE& mapa, VVC& contingut, const set<int>& units_amigues)
   {
     mapa.resize(rows());
     contingut.resize(rows());
     Cell temp_cell;
     for (int i = 0; i < rows(); ++i) {
       mapa[i].resize(cols());
       contingut[i].resize(cols());
       for (int j = 0; j < cols(); ++j) {
         temp_cell = cell(i, j);
         mapa[i][j] = temp_cell.type;
         if(temp_cell.mask) contingut[i][j] = MASCARA;
         else if (in_set(temp_cell.unit_id, units_amigues)) contingut[i][j] = AMIC;
         else if (temp_cell.unit_id != -1) contingut[i][j] = ENEMIC;
         else contingut[i][j] = NO;
       }
     }
   }

   // Calls the find closer funcition, this sub function allow us to know the first step we should do
   Dir sub_find_closer(const VVE& mapa, const Pos& pos_u, Pos& pos_objectiu)
   {
     VVB visitat(mapa.size(), VB(mapa[0].size(), false));
     queue <pair <pair<int, int>, Dir >> q;
     visitat[pos_u.i][pos_u.j] = true;
     int x = pos_u.i;
     int y = pos_u.j;
     if (x != 0 and mapa[x-1][y] != WALL)                      q.push(make_pair(make_pair(pos_u.i-1, pos_u.j), TOP));
     if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL)     q.push(make_pair(make_pair(pos_u.i+1, pos_u.j), BOTTOM));
     if (y != 0 and mapa[x][y-1] != WALL)                      q.push(make_pair(make_pair(pos_u.i, pos_u.j-1), LEFT));
     if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL)  q.push(make_pair(make_pair(pos_u.i, pos_u.j+1), RIGHT));
     pair<Pos,Dir> r = find_closer(mapa, pos_u, q, visitat);
     //if (r.second == NONE) cerr << "ERROR EN EL PATHFINDIG" << endl;
     pos_objectiu = r.first;
     return r.second;
   }

   // Returns the position of the closest path or city
   pair<Pos,Dir> find_closer(const VVE& mapa, const Pos& p, queue <pair <pair<int, int>, Dir >>& q, VVB& visitat)
   {
      Cell temp_cell;
      int x, y;
      while (!q.empty()) {
        x = q.front().first.first;
        y = q.front().first.second;
        Dir temp_dir = q.front().second;
        q.pop();

        if (mapa[x][y] == 2 or mapa[x][y] == 3)
        {
          temp_cell = cell(x,y);
          if (mapa[x][y] == 2 and city_owner(temp_cell.city_id) != me()) return make_pair(Pos(x,y), temp_dir);
          else if (mapa[x][y] == 3 and path_owner(temp_cell.path_id) != me()) return make_pair(Pos(x,y), temp_dir);
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != WALL)                     q.push(make_pair(make_pair(x-1, y), temp_dir));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL)    q.push(make_pair(make_pair(x+1, y), temp_dir));
          if (y != 0 and mapa[x][y-1] != WALL)                     q.push(make_pair(make_pair(x, y-1), temp_dir));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL) q.push(make_pair(make_pair(x, y+1), temp_dir));
        }
      }
      return make_pair(Pos(-1,-1), NONE);
   }

   // Return true if there's an enemy near Pos p, otherwise returns falses
   void enemic_aprop(Pos p, const VVC& contingut, Dir& next_step)
   {
     int x = p.i;
     int y = p.j;
     if (contingut[x-1][y] == ENEMIC) next_step = TOP;
     else if (contingut[x+1][y] == ENEMIC) next_step = BOTTOM;
     else if (contingut[x][y-1] == ENEMIC) next_step = LEFT;
     else if (contingut[x][y+1] == ENEMIC) next_step = RIGHT;
     else if(contingut[x-1][y] == MASCARA) next_step = TOP;
     else if(contingut[x+1][y] == MASCARA) next_step = BOTTOM;
     else if(contingut[x][y-1] == MASCARA) next_step = LEFT;
     else if(contingut[x][y+1] == MASCARA) next_step = RIGHT;
   }

   void move_unitats() {
     VVE mapa;
     VVC contingut;
     VI unitats = my_units(me());
     set<int> set_unitats;
     for (int i : unitats) set_unitats.insert(i);
     obtain_all_data(mapa, contingut, set_unitats);
     Unit unitat;
     Dir next_step = NONE;
     Pos pos_objectiu;
     for (int i = 0; i < (int)unitats.size(); ++i)
     {
       next_step = NONE;
       unitat = unit(unitats[i]);
       enemic_aprop(unitat.pos, contingut, next_step);
       if (next_step == NONE) next_step = sub_find_closer(mapa, unitat.pos, pos_objectiu);
       else; //cerr << "Enemy detected" << endl;
       move(unitat.id, next_step);
       //cerr << "La unitat amb id: " << unitat.id << " es mou a " << next_step << endl;
     }
   }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
    double st = status(me());
    if (st >= 0.9) cerr << "NOOOOO MORRRE CPU TIMMMMMMEEEE" << endl;
    else move_unitats();
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
