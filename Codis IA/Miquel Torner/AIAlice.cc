#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Alice


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

   typedef vector<int> VE;
   typedef vector<VE> VVE;

   /////////// Funcions ////////////////

   // Returns all the data into the 'mapa'
   void obtain_all_data(VVE& mapa)
   {
     mapa.resize(rows());
     Cell temp_cell;
     for (int i = 0; i < rows(); ++i) {
       mapa[i].resize(cols());
       for (int j = 0; j < cols(); ++j) {
         temp_cell = cell(i, j);
         mapa[i][j] = temp_cell.type;
       }
     }
   }

   // Only returns the map around p radi tiles
   void obtain_part_data(VVE& mapa, Pos p, int radi)
   {
     mapa.resize(rows());
     Cell temp_cell;
     int ini_i = p.i - radi, ini_j = p.j - radi;
     int fi_i = p.i + radi, fi_j = p.j + radi;
     for (int i = ini_i - radi; i < fi_i; ++i) {
       mapa[i].resize(cols());
       for (int j = ini_j; j < fi_j; ++j) {
         if(pos_ok(i, j)) {
           temp_cell = cell(i, j);
           mapa[i][j] = temp_cell.type;
         }
         else mapa[i][j] = 0;
       }
     }
   }

   // Returns the position of the closest path or city
   Pos find_closer(const VVE& mapa, Pos p)
   {
     VVE visitat(mapa.size(), VE(mapa[0].size(), false));
     queue <pair <int, int>> q;
     q.push(make_pair(p.i, p.j));
     Cell temp_cell;
     int x, y;
      while (!q.empty()) {
        x = q.front().first;
        y = q.front().second;
        q.pop();

        if (mapa[x][y] == 2 or mapa[x][y] == 3)
        {
          temp_cell = cell(x,y);
          if (mapa[x][y] == 2 and city_owner(temp_cell.city_id) != me()) return Pos(x,y);
          else if (mapa[x][y] == 3 and path_owner(temp_cell.path_id) != me()) return Pos(x,y);
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != '0') q.push(make_pair(x-1, y));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != '0') q.push(make_pair(x+1, y));
          if (y != 0 and mapa[x][y-1] != '0') q.push(make_pair(x, y-1));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != '0') q.push(make_pair(x, y+1));
        }
      }
      return Pos(-1,-1);
   }

   void move_unitats() {
     VVE mapa;
     obtain_all_data(mapa);
     VE unitats = my_units(me());
     Unit unitat;
     for (int i = 0; i < (int)unitats.size(); ++i)
     {
       unitat = unit(unitats[i]);
       Pos objectiu = find_closer(mapa, unitat.pos);
       if (unitat.pos.i < objectiu.i) move(unitat.id, BOTTOM);
       else if (unitat.pos.i > objectiu.i) move(unitat.id, TOP);
       else if (unitat.pos.j < objectiu.j) move(unitat.id, RIGHT);
       else if (unitat.pos.j > objectiu.j) move(unitat.id, LEFT);
     }
   }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
    move_unitats();
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
