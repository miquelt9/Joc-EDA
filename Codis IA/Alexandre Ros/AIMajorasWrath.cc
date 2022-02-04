#include "Player.hh"
#include <queue>
#include <vector>
#include <map>
#include <iostream>

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME MajorasWrath


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

   typedef vector<vector<CellType>> Board;
   Board board;


  void read_board(Board& B){
    for (int j = 0; j < rows(); ++j){
      B.push_back(vector<CellType>());
      for (int i = 0; i < cols(); ++i){
        Cell c = cell(Pos(j, i));
        B[j].push_back(c.type);
      }
    }
  }


  // RETURN VALUES:
  //      (-1, -1) if it's a WALL or an OUT OF RANGE CELL
  //      (0, 0) if it's GRASS without mask.
  //      (0, 1) if it's GRASS WITH MASK
  //      (1, city_id) if it has a CITY with ID city_id
  //      (2, path_id) if it has a PATH with ID path_id
  pair<int, int> valid_cell(int i, int j, const Board& B){

    if (i < 0 or i >= rows() or j < 0 or j >= cols()) return make_pair(-1, -1);

    Cell zz = cell(Pos(j, i));
    CellType z = B[j][i];

    int uid = zz.unit_id;
    if (uid != -1){
      for (int i = 0; i < my_units(me()).size(); ++i)
      {
        if (my_units(me())[i] == uid) return make_pair(-1, -1);
      }
    }

    //if (z == CITY or z == PATH or z == WALL) cout << "La cel·la a la posició col" << i << ", row" << j << "és de tipus ";
    if (z == WALL){
      return make_pair(-1, -1);
    }
    if (z == GRASS){
      if (zz.mask) return make_pair(0,1);
      return make_pair(0, 0);
    }
    if (z == CITY){
      return make_pair(1, zz.city_id);
    }
    if (z == PATH){
      return make_pair(2, zz.path_id);
    }
    return make_pair(-1, -1);

  }


  pair<Dir, int> bfs_search(int i, int j, const Board& B, bool avoid_active){
    // i, j is the position of the unit being studied
    // IMPORTANT !!! AVOID MODE WILL ONLY TAKE EFFECT IF AND ONLY IF avoid_active IS SET TO TRUE
    // WITH AVOID ACTIVE ALL CURRENT CITIES AND PATHS THAT BELONG TO ME WILL BE IGNORED
    // Cell (i, j) is represented by the number j*cols() + i
    int ij = j*cols()+i;

    // Cercarem per amplada amb una profunditat (distància) màxima de 'prof'
    int n = rows()*cols();

    vector<int> prv(n);
    // Taula de previs per a fer la ruta al final
    queue<int> Q;
    Q.push(ij);
    vector<bool> vis(n, false);

    vis[ij] = true;

    prv[ij] = -1;
    while (not Q.empty()){
      int u = Q.front();
      Q.pop();
      int i_2 = u % cols();
      int j_2 = u / cols();

      // Now to try the four different directions
      vector<vector<int>> four_d = {{i_2+1, j_2}, {i_2-1, j_2}, {i_2, j_2+1}, {i_2, j_2-1}};

      for (int d = 0; d < 4; ++d){
        int d_ext = four_d[d][1]*cols()+four_d[d][0];
        if (vis[d_ext]) continue;

        pair<int, int> ret = valid_cell(four_d[d][0], four_d[d][1], B);
        if (ret.first == -1) continue;


        vis[d_ext] = true;
        prv[d_ext] = u;
        bool do_i_own_this = false;

        if (ret.first == 1 or ret.first == 2){
          if (ret.first == 1){
             do_i_own_this = (city_owner(ret.second) == me());
          } else {
             do_i_own_this = (path_owner(ret.second) == me());
          }
        }

        if (ret.first == 0 or (avoid_active and do_i_own_this)){
          Q.push(d_ext);
          continue;
        } else if ((ret.first == 1 or ret.first == 2) and not (do_i_own_this and avoid_active)){
          // City or path found!
          Dir p;
          int x = d_ext;
          int l = 1;
          while (x != -1){
              if (x - prv[x] == 1) p = RIGHT;
              else if (x - prv[x] == -1) p = LEFT;
              else if (x - prv[x] == cols()) p = BOTTOM;
              else if (x - prv[x] == -cols()) p = TOP;
              ++l;
              x = prv[x];
          }
          /*cout << "We have the unit at position column" << i << ", row" << j << endl;
          cout << "His journey will be" << endl;
          for (int i = 0; i < p.size(); ++i){
            if (p[i] == LEFT) cout << "L";
            else if (p[i] == RIGHT) cout << "R";
            else if (p[i] == TOP) cout << "U";
            else if (p[i] == BOTTOM) cout << "D";
          }
          cout << endl;*/
          return make_pair(p, l);

        }
      }


    }
    return make_pair(NONE, 0);
  }

  Dir enemy_ahead(int id){
    Unit u = unit(id);
    Cell z = cell(u.pos);
    for (int d = 0; d < 4; ++d){
      Cell newc = cell(u.pos + Dir(d));
      int e = newc.unit_id;
      Unit eu = unit(e);
      if (e == -1) continue;
      else {
        // Test if the unit 'e' is an enemy or a friend
        bool is_enemy = true;
        vector<int> my_u = my_units(me());
        for (int p = 0; p < my_u.size(); ++p){
          if (my_u[p] == e){
            is_enemy = false;
            break;
          }
        }
        if (is_enemy) return Dir(d);
        else continue;
        // Unit is an enemy!
      }
    }

    return NONE;
  }

  void move_inside_citypath(int id){
    Unit u = unit(id);
    for (int d = 0; d < 4; ++d){
      Cell newc = cell(u.pos + Dir(d));
      if (newc.type == PATH or newc.type == CITY) move(id, Dir(d));
      break;
    }

    move(id, NONE);
  }

  void move_unit(int id){
    Unit u = unit(id);
    Cell z = cell(u.pos);


    vector<int> m = random_permutation(4);
    for (int d = 0; d < 4; ++d){
      Cell newc = cell(u.pos + Dir(m[d]));
      if (newc.mask and not u.mask){
        move(id, Dir(m[d]));
        return;
      }
    }


    // Are you in a city or in a path?
    if (z.type == CITY or z.type == PATH){
      Dir enem = enemy_ahead(id);
      if (enem != NONE){
        move(id, enem);
        return;
      } // Is there an enemy adjacent? If so, attack!



      vector<int> m = random_permutation(4);
      /*for (int d = 0; d < 4; ++d){
        Cell newc = cell(u.pos + Dir(m[d]));
        if (newc.type == PATH){
          int pid = newc.path_id;
          if (path_owner(pid) != me()){
             move(id, Dir(m[d]));
             return;
          }
        }
        if (newc.type == CITY){
          int cid = newc.city_id;
          if (city_owner(cid) != me()){
            move(id, Dir(m[d]));
            return;
          }
        }
      } // Are you adjacent to a city/path who's owner is not you? Move there!
*/

      if (z.type == CITY){
        int cid = z.city_id;
        if (city_owner(cid) == me()){
          pair<Dir, int> s = bfs_search(u.pos.j, u.pos.i, board, true);
          Dir new_set_moves = s.first;
          if (s.second <= 200 - round()) move(id, new_set_moves);
          else move_inside_citypath(id);
          return;
        }
      } // Are you in a city but it's YOUR city? Go to a different path/city.

      if (z.type == PATH){
        int pid = z.path_id;
        if (path_owner(pid) == me()){
          pair<Dir, int> s = bfs_search(u.pos.j, u.pos.i, board, true);
          Dir new_set_moves = s.first;
          if (s.second <= 200 - round()) move(id, new_set_moves);
          else move_inside_citypath(id);
          return;
        }
      } // Are you in a city but it's YOUR city? Go to a different path/city!


      for (int d = 0; d < 4; ++d){
        Cell newc = cell(u.pos + Dir(m[d]));
        if (newc.type == CITY or newc.type == PATH) move(id, Dir(m[d]));
      } // If you're in a city/path then just move randomly within a city/path

    }

    else {
      pair<Dir, int> new_set_moves = bfs_search(u.pos.j, u.pos.i, board, true);
      move(id, new_set_moves.first);
      return;
    } // Is there no predifined move set and you're in no city/path? Search for one!

  }


  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
    if (round() == 0) read_board(board);
    vector<int> units = my_units(me());
    int n = units.size();

    for (int i = 0; i < n; ++i){
      // Bucle per a cada unitat
      move_unit(units[i]);
    }

  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
