#include "Player.hh"
#include <set>
#include <map>

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME MyDummy


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
     ENEMIC,        // Indica que hi ha un enemic a la casella
     AMIC,          // Indica que hi ha un amic a la casella
     MASCARA,       // Indica que hi ha una mascara a la casella
     EASY_KILL,     // Indica que un enemic fa temps_easy_kill rondes que esta al mateix lloc amb un moviment maxim de temps_estacionari
     OCUPADA,       // Indica que hi ha un amic ocuparà la casella i per tant no estarà disponible
     NO             // Per defecte, no hi ha res
   };

   typedef vector<int> VI;
   typedef vector<VI> VVI;
   typedef vector<bool> VB;
   typedef vector<VB> VVB;
   typedef vector<CellType> VE;
   typedef vector<VE> VVE;
   typedef vector<Casella> VC;
   typedef vector<VC> VVC;
   typedef vector<pair<int,int>> VP;
   typedef vector<VP> VVP;

   VVE mapa;                      // Mapa (conté la posició de Grass, City, Path i Wall)
   VVC contingut;                 // Indica que hi ha a cada ronda a cada Casella (veure descripció del tipus)
   VVP enemies;                   // S'utilitza per tenir controlats els moviments de l'enemic i saber si la seva posició serà esperable
   set<int> set_unitats;          // Set de les unitats amigues (copiat del vector)
   vector<pair<int,Dir>> pre_moviments;    // Moviments abans de tractar-ho

   vector<set<int>> assignacions_ciutats;
   vector<set<int>> assignacions_camins;
   set<int> ciutats_bloquejades;
   set<int> camins_bloquejades;

   vector<set<int>> campers_ciutats;   // Cada posició fa referència a cada ciutat i el set indicara les unitats que hi campejen
   vector<int> max_campers_ciutats;    // Indicarà el nº màxim de unitats que han d'estar a una ciutat
   set<pair<int,int>> ciutats_ordenades_mida; // Realment es temporal per ordenar les ciutats per mida i després ficar-ho a la cua
   queue<int> ciutats_ordenades;      // Cua ordenada per ordre de ciutat gran a petita
   map<int, pair<Pos,Pos>> camins_ciutats; // Conté la posició de cadascun dels camins que connecta amb la ciutat (realment no s'utilitza xd)

   VI unitats;                    // Unitats amigues
   int n_rows;                    // Número de files
   int n_cols;                    // Número de columnes

   bool easy_kill = false;
   int temps_estacionari = 1;        // Rondes que han de passar abans que un enemic sigui considerat que ha abandonat la posició (nota que si =1 llavors l'enemic es pot moure i tornar a la pos anterior i serà considerat com si no s'hagués mogut)
   int temps_easy_kill = 3;          // Rondes que un enemic ha d'estar a la mateixa posició perquè es consideri que es facil de matar ja que no es mourà
   int distancia_max_easy_kill = 10;

   bool kamikaze = true;
   int distancia_max_kamikaze = 15;  // Distància màxima que un kamikaze atacarà
   int rounds_kamikaze = 150;        // Rondes fins les que els kamikazes estaràn actius, després es comportaran com unitats normals

   int distancia_max_bfs = 60;       // Distància fins la qual es farà el bfs, sinó s'ha trobat res pararà
   int vida_fugir = 20;               // Si la vida de la unitat és inferior evitarà els enemics
   //int vida_atacar = 30;           // Si la vida és superior atacarà

   bool assignacions = true;
   int grup_maxim_ciutat = 5;       // Número màxim d'unitats que atacaran a una ciutat
   int grup_minim_ciutat = 1;
   int grup_maxim_cami = 4;         // Número màxim d'unitats que atacaran a un camí
   int grup_minim_cami = 1;

   bool limit = true;                  // Si està a false seria equivalent a ficara INF als dos atributs de sota (pero evita consum de CPU)
   int unitats_no_atacar_ciutat = 5;       // Si hi ha 3 o més unitats en una ciutat no atacarem i anirem a buscar altres opcions
   int unitats_no_atacar_cami = 5;       // Si hi ha 3 o més unitats en un cami no atacarem i anirem a buscar altres opcions

   bool campejar = false;
   int grup_campejar = 1;           // Unitats màximes que es quedaràn a campejar (-1 per assignar automaticament)
   int unitats_lliures = nb_units();         // Mai campejaran

   /////////// Funcions ////////////////

   // Checks wheater a int is in the set
   bool es_amiga(int id)
   {
     return (set_unitats.find(id) != set_unitats.end());
   }

   bool in_set(int x, set<int> this_set)
   {
     return (this_set.find(x) != this_set.end());
   }

   // Given a position and a dirrection, returns true if the resulting pos is OCUPADA, false otherwise
   bool ocupada(const Pos& pos_u, const Dir& dir)
   {
     int x = pos_u.i, y = pos_u.j;
     if (dir == TOP) --x;
     else if(dir == BOTTOM) ++x;
     else if(dir == LEFT) --y;
     else if(dir == RIGHT) ++y;
     return contingut[x][y] == OCUPADA;
   }

   bool distancia_ok(const Pos& pos_u, int x, int y, int distancia)
   {
     return (abs(x-pos_u.i) + abs(y-pos_u.j)) < distancia;
   }

   bool is_city_or_path(const Pos& pos_u)
   {
     return is_city(pos_u.i, pos_u.j) or is_path(pos_u.i, pos_u.j);
   }

   bool is_city_or_path(int x, int y)
   {
     return is_city(x,y) or is_path(x,y);
   }

   bool is_city(const Pos& pos_u)
   {
     return is_city(pos_u.i, pos_u.j);
   }

   bool is_city(int x, int y)
   {
     return mapa[x][y] == CITY;
   }

   bool is_path(const Pos& pos_u)
   {
     return is_path(pos_u.i, pos_u.j);
   }

   bool is_path(int x, int y)
   {
     return mapa[x][y] == PATH;
   }

   // Returns true if the unitat can join (then inserts it into the apropiate set) or alredy is in the group that is going to the city
   bool city_assignada(int unitat_id, int ciutat_id)
   {
     if (assignacions_ciutats[ciutat_id].find(unitat_id) != assignacions_ciutats[ciutat_id].end()) return true;
     if ((int)assignacions_ciutats[ciutat_id].size() >= grup_maxim_ciutat) return false;
     assignacions_ciutats[ciutat_id].insert(unitat_id); return true;
   }

   // Returns true if the unitat can join (then inserts it into the apropiate set) or alredy is in the group that is going to the city
   bool path_assignada(int unitat_id, int cami_id)
   {
     if (assignacions_camins[cami_id].find(unitat_id) != assignacions_camins[cami_id].end()) return true;
     if ((int)assignacions_camins[cami_id].size() >= grup_maxim_cami) return false;
     assignacions_camins[cami_id].insert(unitat_id); return true;
   }

   bool assignar_campejar(Unit unitat)
   {
     //cerr << "getting city id for camp" << endl;
     int ciutat_id = cell(unitat.pos).city_id;

     //cerr << "city id got " << ciutat_id << endl;
     if (ciutat_id == -1) return false;

     int maxim_unitats;
     if (grup_campejar == -1) maxim_unitats = max_campers_ciutats[ciutat_id];
     else maxim_unitats = grup_campejar;

     if (campers_ciutats[ciutat_id].find(unitat.id) != campers_ciutats[ciutat_id].end()){
      //cerr << "already assgined" << endl;
      return true;}
     if ((int)campers_ciutats[ciutat_id].size() >= maxim_unitats){
        //cerr << "group aleary full" << endl;
        return false;
      }
     campers_ciutats[ciutat_id].insert(unitat.id);
     //cerr << "unitat assiganada camping unit_id/city_id " << unitat.id << " " << ciutat_id << endl;
     return true;
   }

   bool camping(int unitat_id)
   {
     for (int i = 0; i < (int)campers_ciutats.size(); ++i)
     {
       if (campers_ciutats[i].find(unitat_id) != campers_ciutats[i].end()) return true;
     }
     return false;
   }

   bool cami_conquerit(Pos p)
   {
     return city_owner(cell(p).path_id) == me();
   }

  /////////////////////////////// OBTAIN DATA //////////////////////////////////

   // Returns all the data into the 'mapa'
   void obtain_content_data()
   {
     Cell temp_cell;
     for (int i = 0; i < n_rows; ++i) {
       for (int j = 0; j < n_cols; ++j) {
         temp_cell = cell(i, j);
         if (temp_cell.mask) contingut[i][j] = MASCARA;
         else if (es_amiga(temp_cell.unit_id)) contingut[i][j] = AMIC;
         else if (temp_cell.unit_id != -1)
         {
           if (not easy_kill) contingut[i][j] = ENEMIC;
           else
           {
             if (enemies[i][j].first == (temp_cell.unit_id)) ++enemies[i][j].second;    // Utilitzem el second com a contador per saber quantes rondes fa que no es mou
             else {enemies[i][j].first = temp_cell.unit_id; enemies[i][j].second = 0;} // Si hi ha una unitat diferent a la que hi havia fiquem el contador a 0
             if (enemies[i][j].second > temps_easy_kill) contingut[i][j] = EASY_KILL;
             else contingut[i][j] = ENEMIC;
           }
         }
         else
         {
           // Ens permetra trobar els jugadors que es mouen sempre ens les mateixes caselles (amunt-avall p.e.) i detectarlos com estàtics
           if (easy_kill)
           {
             if (round() + enemies[i][j].second > temps_estacionari) {enemies[i][j].first = -1; enemies[i][j].second = 0;} // Si ha passat mes de n rondes es considerara que la unitat ha marxat
             else if (enemies[i][j].second >= 0) enemies[i][j].second = -round(); // Utilizarem la ronda en negatiu (només s'actualitza 1 vegada per enemic i casella) per saber quantes rondes fa que havia estat al lloc
           }
           contingut[i][j] = NO;
         }
       }
     }
   }

   // Obtain data from the map (grass, cities, paths and walls)
   void obtain_all_map_data()
   {
     Cell temp_cell;
     for (int i = 0; i < n_rows; ++i) {
       for (int j = 0; j < n_cols; ++j) {
         temp_cell = cell(i, j);
         mapa[i][j] = temp_cell.type;
       }
     }
   }

   void check_assignacions()
   {
     set<int> set_temp;
     //cerr << "abans cities" << endl;
     for (int i = 0; i < nb_cities(); ++i)
     {
       set_intersection(assignacions_ciutats[i].begin(), assignacions_ciutats[i].end(),
                      set_unitats.begin(), set_unitats.end(),
                      inserter(set_temp, set_temp.begin()));
       assignacions_ciutats[i] = set_temp;
       set_temp.clear();
     }
     //cerr << "despres cities" << endl;
     for (int i = 0; i < nb_paths(); ++i)
     {
       set_intersection(assignacions_camins[i].begin(), assignacions_camins[i].end(),
                      set_unitats.begin(), set_unitats.end(),
                      inserter(set_temp, set_temp.begin()));
       assignacions_camins[i] = set_temp;
       set_temp.clear();
     }

   }

   void check_campers()
   {
     set<int> set_temp;
     for (int i = 0; i < nb_cities(); ++i)
     {
       set_intersection(campers_ciutats[i].begin(), campers_ciutats[i].end(),
                      set_unitats.begin(), set_unitats.end(),
                      inserter(set_temp, set_temp.begin()));
       campers_ciutats[i] = set_temp;
       set_temp.clear();
     }
   }

   //Simply calls other funcitions to obtain round data
   void obtain_round_data()
   {
     unitats = my_units(me());
     set_unitats.clear();
     for (int i : unitats) set_unitats.insert(i);
     if (round() != 0)
     {
       if (assignacions) check_assignacions();
       if (campejar)
       {
         check_campers();
         indicar_campejar(ciutats_ordenades);
       }
     }
     if (round() == 0)
     {
       resize_matrix();
       obtain_all_map_data();
       if (campejar)
       {
         calcular_ciutats();
         calcular_camins();
       }
     }
     obtain_content_data();

     if (assignacions or limit)
     {
       ciutats_bloquejades.clear();
       camins_bloquejades.clear();
       if (limit) bloquejar_ciutats_camins();
     }
   }

   /////////////////////////////////////////////////////////////////////////////

   void bloquejar_ciutats_camins()
   {
     for (int i = 0; i < nb_cities(); ++i)
     {
       if (unitats_enemigues_ciutat(i, true) >= unitats_no_atacar_ciutat) ciutats_bloquejades.insert(i);
     }
     for (int i = 0; i < nb_paths(); ++i)
     {
       if (unitats_enemigues_cami(i) >= unitats_no_atacar_cami) camins_bloquejades.insert(i);
     }
   }

   // Resize the matrixs so this process just have to be done once
   void resize_matrix()
   {
     assignacions_ciutats.clear();
     assignacions_ciutats.resize(nb_cities());
     assignacions_camins.clear();
     assignacions_camins.resize(nb_paths());
     campers_ciutats.clear();
     campers_ciutats.resize(nb_cities());
     max_campers_ciutats.clear();
     max_campers_ciutats.resize(nb_cities());

     n_rows = rows();
     n_cols = cols();
     mapa.resize(n_rows);
     contingut.resize(n_rows);
     enemies.resize(n_rows);
     for (int i = 0; i < n_rows; ++i) {
       mapa[i].resize(n_cols);
       contingut[i].resize(n_cols);
       enemies[i].resize(n_cols);
     }
   }

   void indicar_campejar(queue<int> q)
   {
     int fi = my_units(me()).size();

     for (int i = 0; i < (int)max_campers_ciutats.size(); ++i) max_campers_ciutats[i] = 0;

     for (int i = 0; i < fi - unitats_lliures; ++i)
     {
       int temp_city = q.front();
       max_campers_ciutats[temp_city]++;
       q.pop();
       q.push(temp_city);
     }
   }

   void calcular_ciutats()
   {
     for (int i = 0; i < nb_cities(); ++i)
     {
       int mida = city(i).size();
       ciutats_ordenades_mida.insert(make_pair(mida,i));
     }

     for (auto it = ciutats_ordenades_mida.end(); it != ciutats_ordenades_mida.begin(); --it)
     {
       ciutats_ordenades.push(it->second);
     }
     ciutats_ordenades.push(ciutats_ordenades_mida.begin()->second);
   }

   void calcular_camins()
   {
     camins_ciutats.clear();
     Pos p; p.i =-1; p.j=-1;

     for (int i = 0; i < nb_paths(); ++i)
     {
       camins_ciutats[i] = make_pair(p, p);
     }
     int temp_city_id;
     Path cami;
     Pos temp_pos;
     for (int i = 0; i < nb_paths(); ++i)
     {
       cami = path(i);
       temp_pos = cami.second[0];
       temp_city_id = get_adj_city_id(temp_pos);
       if (camins_ciutats[temp_city_id].first == p) camins_ciutats[temp_city_id].first = temp_pos;
       else                                         camins_ciutats[temp_city_id].second = temp_pos;

       temp_pos = cami.second[cami.second.size()-1];
       temp_city_id = get_adj_city_id(temp_pos);
       if (camins_ciutats[temp_city_id].first == p) camins_ciutats[temp_city_id].first = temp_pos;
       else                                         camins_ciutats[temp_city_id].second = temp_pos;
     }
   }

   // Set the 'contingut' to OCUPADA so other friendly units know where it's moving to
   void indicar_casella_ocupada(Pos p, Dir next_step)
   {
     if (next_step == TOP)          contingut[p.i-1][p.j] = OCUPADA;
     else if (next_step == BOTTOM)  contingut[p.i+1][p.j] = OCUPADA;
     else if (next_step == LEFT)    contingut[p.i][p.j-1] = OCUPADA;
     else if (next_step == RIGHT)   contingut[p.i][p.j+1] = OCUPADA;
     else if (next_step == NONE)    contingut[p.i][p.j] = OCUPADA;
   }

   // Si hi ha algun enemic que pugui interferir al nostre moviment es marcaran les caselles com a visitades per evitar que la unitat hi vagi
   void marcar_enemics_com_visitats(VVB& visitat, const Pos& pos_u)
   {
     int i = pos_u.i, j = pos_u.j;
     //bool no_segmentation_fault = (i > 1 and i < n_rows-2 and j > 1 and j < n_cols-2);
     visitat[i+1][j] = (contingut[i+1][j] == ENEMIC);
     // visitat[i+1][j] = (contingut[i+1][j] == ENEMIC or contingut[i+1][j+1] == ENEMIC or contingut[i+1][j-1] == ENEMIC or (no_segmentation_fault and contingut[i+2][j] == ENEMIC)); // BOTTOM
     visitat[i-1][j] = (contingut[i-1][j] == ENEMIC);
     // visitat[i-1][j] = (contingut[i-1][j] == ENEMIC or contingut[i-1][j-1] == ENEMIC or contingut[i-1][j+1] == ENEMIC or (no_segmentation_fault and contingut[i-2][j] == ENEMIC)); // TOP
     visitat[i][j+1] = (contingut[i][j+1] == ENEMIC);
     // visitat[i][j+1] = (contingut[i][j+1] == ENEMIC or contingut[i-1][j+1] == ENEMIC or contingut[i+1][j+1] == ENEMIC or (no_segmentation_fault and contingut[i][j+2] == ENEMIC)); // RIGHT
     visitat[i][j-1] = (contingut[i][j-1] == ENEMIC);
     // visitat[i][j-1] = (contingut[i][j-1] == ENEMIC or contingut[i-1][j-1] == ENEMIC or contingut[i+1][j-1] == ENEMIC or (no_segmentation_fault and contingut[i][j-2] == ENEMIC)); // LEFT
   }

   void eliminar_assignacio(Unit unitat, Pos pos_objectiu)
   {
     //if (not (pos_ok(pos_objectiu) or pos_ok(unitat.pos))) return;
     Cell temp_cell_ini = cell(unitat.pos);
     Cell temp_cell_fi = cell(pos_objectiu);
     if (temp_cell_ini.city_id != temp_cell_fi.city_id or temp_cell_ini.path_id != temp_cell_fi.path_id)
     {
       if (temp_cell_fi.type == CITY and city_owner(temp_cell_fi.city_id) == me()) assignacions_ciutats[temp_cell_fi.city_id].erase(unitat.id);
       if (temp_cell_fi.type == PATH and path_owner(temp_cell_fi.path_id) == me()) assignacions_camins[temp_cell_fi.path_id].erase(unitat.id);
     }
   }

   set<int> eliminar_assignacio_insuficient()
   {
     int i = 0, j = nb_cities();
     set<int> unitats_rectificar;

     while (i < j)
     {
       for (auto s : assignacions_ciutats)
       {
         int tamany = (int)s.size();
         if (tamany > 0 and tamany < grup_minim_ciutat) {
           ciutats_bloquejades.insert(i);
           for (auto k : s) unitats_rectificar.insert(k);
           assignacions_ciutats[i].clear();
         }
       }
       ++i;
     }

     i = 0; j = nb_paths();

     while (i < j)
     {
       for (auto s : assignacions_camins)
       {
         int tamany = (int)s.size();
         if (tamany > 0 and tamany < grup_minim_cami) {
           camins_bloquejades.insert(i);
           for (auto k : s) unitats_rectificar.insert(k);
           assignacions_camins[i].clear();
         }
       }
       ++i;
     }

     return unitats_rectificar;
   }

   void eliminar_campejar()
   {
     for (int i = 0; i < (int)nb_cities(); ++i)
     {
       while ((int)campers_ciutats[i].size() > max_campers_ciutats[i])
       {
         campers_ciutats[i].erase(campers_ciutats[i].begin());
       }
     }
   }

   CellType future_pos_type(Dir dir, int x, int y)
   {
     Cell temp_cell;
     if (dir == TOP) x--;
     else if (dir == BOTTOM) x++;
     else if (dir == LEFT) y--;
     else if (dir == RIGHT) y++;
     temp_cell = cell(x, y);
     //cerr << "Future Cell " << x << " " << y << temp_cell.type << endl;
     return temp_cell.type;
   }

   Dir direccio(int i)
   {
     if (i == 0) return BOTTOM;
     if (i == 1) return RIGHT;
     if (i == 2) return TOP;
     if (i == 3) return LEFT;
     return NONE;
   }

   int unitats_enemigues_adjacents(const Pos& pos_u)
   {
     int count = 0;
     int i = pos_u.i, j = pos_u.j;
     if (contingut[i+1][j] == ENEMIC) ++count;
     if (contingut[i-1][j] == ENEMIC) ++count;
     if (contingut[i][j+1] == ENEMIC) ++count;
     if (contingut[i][j-1] == ENEMIC) ++count;
     return count;
   }

   int unitats_enemigues_ciutat(int ciutat_id, bool incluir_adj)
   {
     vector<Pos> ciutat = city(ciutat_id);
     int ini_i = ciutat[0].i, ini_j = ciutat[0].j;
     int fi_i = ciutat[ciutat.size()-1].i, fi_j = ciutat[ciutat.size()-1].j;

     if (incluir_adj) {ini_i-=1; ini_j-=1; fi_i+=1; fi_j+=1;}

     Unit temp_unitat;
     Cell temp_cell;
     vector<int> unit_count(nb_players());

     for (int i = ini_i; i < fi_i; i++)
     {
       for (int j = ini_j; j < fi_j; ++j) if (contingut[i][j] == ENEMIC) unit_count[unit(cell(i, j).unit_id).player]++;
     }

     int suma = 0;
     for (int i : unit_count) if (i != me()) suma += i;
     return suma;
   }

   int unitats_enemigues_cami(int cami_id)
   {
     vector<int> unit_count(nb_players());

     Path cami;
     Cell temp_cell;
     Unit temp_unitat;
     for (int i = 0; i < nb_paths(); ++i)
     {
       cami = path(i);
       for (auto p : cami.second) if (contingut[p.i][p.j] == ENEMIC) unit_count[unit(cell(p).unit_id).player]++;
     }

     int suma = 0;
     for (int i : unit_count) if (i != me()) suma += i;
     return suma;
   }

   //////////////////////////// BFS GENERIC ////////////////////////////////////

   // Calls the find closer funcition, this sub function allow us to know the first step we should do
   Dir sub_find_closer(const Unit& unitat, Pos& pos_objectiu, bool inside, bool marcar_enemics, bool goto_path)
   {
     VVB visitat(mapa.size(), VB(mapa[0].size(), false));
     queue <pair <pair<int, int>, Dir >> q;
     int x = unitat.pos.i;
     int y = unitat.pos.j;
     //Cell temp_cell = cell(x, y);

     pair<Pos,Dir> r = make_pair(Pos(-1,-1), NONE);
     if (marcar_enemics) marcar_enemics_com_visitats(visitat, unitat.pos);
     visitat[unitat.pos.i][unitat.pos.j] = true;

     if (x != 0 and mapa[x-1][y] != WALL and not visitat[x-1][y])                      q.push(make_pair(make_pair(unitat.pos.i-1, unitat.pos.j), TOP));
     if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL and not visitat[x+1][y])     q.push(make_pair(make_pair(unitat.pos.i+1, unitat.pos.j), BOTTOM));
     if (y != 0 and mapa[x][y-1] != WALL and not visitat[x][y-1])                      q.push(make_pair(make_pair(unitat.pos.i, unitat.pos.j-1), LEFT));
     if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL and not visitat[x][y+1])  q.push(make_pair(make_pair(unitat.pos.i, unitat.pos.j+1), RIGHT));

     // Si només hi ha una sortida per on podem avançar (només hauria de passar si estem envoltats d'enemics(fet rar)) anirem en aquella direcció directament
     if (q.size() == 1) return q.front().second;

     // Prioritzarem moures per dintre de les ciutats i camins per molestar en cas que la vida sigui baixa (nota que tmb esquivara els enemics per una condició anterior)
     // if (unitat.health < vida_fugir) r = find_closer_inside(unitat, q, visitat);
     if (r.second == NONE and goto_path) r = find_closer_path_inside(unitat, q, visitat);
     if (r.second == NONE and not inside) r = find_closer(unitat, q, visitat);
     else if (r.second == NONE and inside) r = find_closer_inside(unitat, q, visitat);
     pos_objectiu = r.first;
     return r.second;
   }

   // Returns the position and the direction of the closest path or city
   pair<Pos,Dir> find_closer(const Unit& unitat, queue <pair <pair<int, int>, Dir >>& q, VVB& visitat)
   {
      Cell temp_cell;
      int x, y;
      while (!q.empty()) {
        x = q.front().first.first;
        y = q.front().first.second;
        Dir temp_dir = q.front().second;
        q.pop();

        if (is_city_or_path(x, y))
        {
          temp_cell = cell(x,y);
          if (is_city(x, y) and city_owner(temp_cell.city_id) != me() and not in_set(temp_cell.city_id, ciutats_bloquejades))
          {
            if (assignacions and city_assignada(unitat.id, temp_cell.city_id)) return make_pair(Pos(x,y), temp_dir);
            else return make_pair(Pos(x,y), temp_dir);
          }

          if (is_path(x, y) and path_owner(temp_cell.path_id) != me() and not in_set(temp_cell.city_id, camins_bloquejades))
          {
            if (assignacions and path_assignada(unitat.id, temp_cell.path_id)) return make_pair(Pos(x,y), temp_dir);
            else return make_pair(Pos(x,y), temp_dir);
          }
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != WALL)                     q.push(make_pair(make_pair(x-1, y), temp_dir));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL)    q.push(make_pair(make_pair(x+1, y), temp_dir));
          if (y != 0 and mapa[x][y-1] != WALL)                     q.push(make_pair(make_pair(x, y-1), temp_dir));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL) q.push(make_pair(make_pair(x, y+1), temp_dir));
        }
        if (not distancia_ok(unitat.pos, x, y, distancia_max_bfs)) return make_pair(Pos(-1,-1), NONE);
      }
      return make_pair(Pos(-1,-1), NONE);
   }

   // Returns the position and the direction of the closest path or city (Treats grass as walls)
   pair<Pos,Dir> find_closer_inside(const Unit& unitat, queue <pair <pair<int, int>, Dir >>& q, VVB& visitat)
   {
      Cell temp_cell;
      int x, y;
      while (!q.empty()) {
        x = q.front().first.first;
        y = q.front().first.second;
        Dir temp_dir = q.front().second;
        q.pop();

        if (is_city_or_path(x, y))
        {
          temp_cell = cell(x,y);
          if (is_city(x, y) and city_owner(temp_cell.city_id) != me() and not in_set(temp_cell.city_id, ciutats_bloquejades))
          {
            if (assignacions and city_assignada(unitat.id, temp_cell.city_id)) return make_pair(Pos(x,y), temp_dir);
            else if (not assignacions) return make_pair(Pos(x,y), temp_dir);
          }

          if (is_path(x, y) and path_owner(temp_cell.path_id) != me() and not in_set(temp_cell.city_id, camins_bloquejades))
          {
            if (assignacions and path_assignada(unitat.id, temp_cell.path_id)) return make_pair(Pos(x,y), temp_dir);
            else if (not assignacions) return make_pair(Pos(x,y), temp_dir);
          }
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != WALL and mapa[x-1][y] != GRASS)                     q.push(make_pair(make_pair(x-1, y), temp_dir));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL and mapa[x+1][y] != GRASS)    q.push(make_pair(make_pair(x+1, y), temp_dir));
          if (y != 0 and mapa[x][y-1] != WALL and mapa[x][y-1] != GRASS)                     q.push(make_pair(make_pair(x, y-1), temp_dir));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL and mapa[x][y+1] != GRASS) q.push(make_pair(make_pair(x, y+1), temp_dir));
        }
        if (not distancia_ok(unitat.pos, x, y, distancia_max_bfs)) return make_pair(Pos(-1,-1), NONE);
      }
       return make_pair(Pos(-1,-1), NONE);
   }

   // Returns the position and the direction of the closest path (Treats grass as walls)
   pair<Pos,Dir> find_closer_path_inside(const Unit& unitat, queue <pair <pair<int, int>, Dir >>& q, VVB& visitat)
   {
      Cell temp_cell;
      int x, y;
      while (!q.empty()) {
        x = q.front().first.first;
        y = q.front().first.second;
        Dir temp_dir = q.front().second;
        q.pop();

        if (is_city_or_path(x, y))
        {
          temp_cell = cell(x,y);
          if (is_path(x, y) and path_owner(temp_cell.path_id) != me() and not in_set(temp_cell.city_id, camins_bloquejades))
          {
            if (assignacions and path_assignada(unitat.id, temp_cell.path_id)) return make_pair(Pos(x,y), temp_dir);
            else if (not assignacions) return make_pair(Pos(x,y), temp_dir);
          }
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != WALL and mapa[x-1][y] != GRASS)                     q.push(make_pair(make_pair(x-1, y), temp_dir));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL and mapa[x+1][y] != GRASS)    q.push(make_pair(make_pair(x+1, y), temp_dir));
          if (y != 0 and mapa[x][y-1] != WALL and mapa[x][y-1] != GRASS)                     q.push(make_pair(make_pair(x, y-1), temp_dir));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL and mapa[x][y+1] != GRASS) q.push(make_pair(make_pair(x, y+1), temp_dir));
        }
        if (not distancia_ok(unitat.pos, x, y, distancia_max_bfs)) return make_pair(Pos(-1,-1), NONE);
      }
       return make_pair(Pos(-1,-1), NONE);
   }

   //////////////////////// BFS FOR ENEMIES ////////////////////////////////////

   // Calls the find closer funcition, this sub function allow us to know the first step we should do
   Dir sub_find_closer_enemy(const Unit& unitat, Pos& pos_objectiu, bool inside, bool only_city)
   {
     VVB visitat(mapa.size(), VB(mapa[0].size(), false));
     queue <pair <pair<int, int>, Dir >> q;
     int x = unitat.pos.i;
     int y = unitat.pos.j;
     //Cell temp_cell = cell(x, y);

     pair<Pos,Dir> r = make_pair(Pos(-1,-1), NONE);
     visitat[unitat.pos.i][unitat.pos.j] = true;

     if (x != 0 and mapa[x-1][y] != WALL and not visitat[x-1][y])                      q.push(make_pair(make_pair(unitat.pos.i-1, unitat.pos.j), TOP));
     if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL and not visitat[x+1][y])     q.push(make_pair(make_pair(unitat.pos.i+1, unitat.pos.j), BOTTOM));
     if (y != 0 and mapa[x][y-1] != WALL and not visitat[x][y-1])                      q.push(make_pair(make_pair(unitat.pos.i, unitat.pos.j-1), LEFT));
     if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL and not visitat[x][y+1])  q.push(make_pair(make_pair(unitat.pos.i, unitat.pos.j+1), RIGHT));

     // Si només hi ha una sortida per on podem avançar (només hauria de passar si estem envoltats d'enemics(fet rar)) anirem en aquella direcció directament
     if (q.size() == 1) return q.front().second;

     if (r.second == NONE and inside and only_city) r = find_closer_enemy_city(unitat, q, visitat);
     else
     {
       if (r.second == NONE and not inside) r = find_closer_enemy(unitat, q, visitat);
       else if (r.second == NONE and inside) r = find_closer_enemy_inside(unitat, q, visitat);
     }
     pos_objectiu = r.first;
     return r.second;
   }

   // Returns the position and the direction of the closest enemy (Treats grass as walls)
   pair<Pos,Dir> find_closer_enemy_inside(const Unit& unitat, queue <pair <pair<int, int>, Dir >>& q, VVB& visitat)
   {
      int x, y;
      while (!q.empty()) {
        x = q.front().first.first;
        y = q.front().first.second;
        Dir temp_dir = q.front().second;
        q.pop();

        if (contingut[x][y] == ENEMIC or contingut[x][y] == EASY_KILL)
        {
          if (distancia_ok(unitat.pos, x, y, distancia_max_kamikaze)) return make_pair(Pos(x,y), temp_dir);
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != WALL and mapa[x-1][y] != GRASS)                     q.push(make_pair(make_pair(x-1, y), temp_dir));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL and mapa[x+1][y] != GRASS)    q.push(make_pair(make_pair(x+1, y), temp_dir));
          if (y != 0 and mapa[x][y-1] != WALL and mapa[x][y-1] != GRASS)                     q.push(make_pair(make_pair(x, y-1), temp_dir));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL and mapa[x][y+1] != GRASS) q.push(make_pair(make_pair(x, y+1), temp_dir));
        }
        if (not distancia_ok(unitat.pos, x, y, distancia_max_kamikaze)) return make_pair(Pos(-1,-1), NONE);
      }
       return make_pair(Pos(-1,-1), NONE);
   }

   // Returns the position and the direction of the closest enemy (Treats grass as walls)
   pair<Pos,Dir> find_closer_enemy_city(const Unit& unitat, queue <pair <pair<int, int>, Dir >>& q, VVB& visitat)
   {
      int x, y;
      while (!q.empty()) {
        x = q.front().first.first;
        y = q.front().first.second;
        Dir temp_dir = q.front().second;
        q.pop();

        if (contingut[x][y] == ENEMIC or contingut[x][y] == EASY_KILL)
        {
          if (distancia_ok(unitat.pos, x, y, distancia_max_kamikaze)) return make_pair(Pos(x,y), temp_dir);
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != WALL and mapa[x-1][y] != GRASS and mapa[x-1][y] != PATH)                     q.push(make_pair(make_pair(x-1, y), temp_dir));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL and mapa[x+1][y] != GRASS and mapa[x+1][y] != PATH)    q.push(make_pair(make_pair(x+1, y), temp_dir));
          if (y != 0 and mapa[x][y-1] != WALL and mapa[x][y-1] != GRASS and mapa[x][y-1] != PATH)                     q.push(make_pair(make_pair(x, y-1), temp_dir));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL and mapa[x][y+1] != GRASS and mapa[x][y+1] != PATH) q.push(make_pair(make_pair(x, y+1), temp_dir));
        }
        if (not distancia_ok(unitat.pos, x, y, distancia_max_kamikaze)) return make_pair(Pos(-1,-1), NONE);
      }
       return make_pair(Pos(-1,-1), NONE);
   }

   // Returns the position and the direction of the closest enemy
   pair<Pos,Dir> find_closer_enemy(const Unit& unitat, queue <pair <pair<int, int>, Dir >>& q, VVB& visitat)
   {
      int x, y;
      while (!q.empty()) {
        x = q.front().first.first;
        y = q.front().first.second;
        Dir temp_dir = q.front().second;
        q.pop();

        if (contingut[x][y] == ENEMIC or contingut[x][y] == EASY_KILL)
        {
          if (distancia_ok(unitat.pos, x, y, distancia_max_kamikaze)) return make_pair(Pos(x,y), temp_dir);
        }
        if (!visitat[x][y]) {
          visitat[x][y] = true;
          if (x != 0 and mapa[x-1][y] != WALL)                     q.push(make_pair(make_pair(x-1, y), temp_dir));
          if (x != (int)mapa.size()-1 and mapa[x+1][y] != WALL)    q.push(make_pair(make_pair(x+1, y), temp_dir));
          if (y != 0 and mapa[x][y-1] != WALL)                     q.push(make_pair(make_pair(x, y-1), temp_dir));
          if (y != (int)mapa[0].size()-1 and mapa[x][y+1] != WALL) q.push(make_pair(make_pair(x, y+1), temp_dir));
        }
        if (not distancia_ok(unitat.pos, x, y, distancia_max_kamikaze)) return make_pair(Pos(-1,-1), NONE);
      }
       return make_pair(Pos(-1,-1), NONE);
   }

   //////////////////// ADJACENT TILE CHECK ////////////////////////////////////

   // Defines next_step if there's an enemy near Pos p, otherwise none
   void atacar_enemic_aprop(Pos p, Dir& next_step, Pos& pos_objectiu)
   {
     int x = p.i;
     int y = p.j;
     if (contingut[x-1][y] == ENEMIC) {next_step = TOP; --pos_objectiu.i;}
     else if (contingut[x+1][y] == ENEMIC) {next_step = BOTTOM; ++pos_objectiu.i;}
     else if (contingut[x][y-1] == ENEMIC) {next_step = LEFT; --pos_objectiu.j;}
     else if (contingut[x][y+1] == ENEMIC) {next_step = RIGHT; ++pos_objectiu.j;}
   }

   // Defines next_step if there's a mask near Pos p, otherwise none
   void mascara_aprop(Pos p, Dir& next_step, Pos& pos_objectiu)
   {
     int x = p.i;
     int y = p.j;
     if(contingut[x-1][y] == MASCARA) {next_step = TOP; --pos_objectiu.i;}
     else if(contingut[x+1][y] == MASCARA) {next_step = BOTTOM; ++pos_objectiu.i;}
     else if(contingut[x][y-1] == MASCARA) {next_step = LEFT; --pos_objectiu.j;}
     else if(contingut[x][y+1] == MASCARA) {next_step = RIGHT; ++pos_objectiu.j;}
   }

   void fugir(Pos p, Dir& next_step, Pos& pos_objectiu)
   {
     int i = p.i, j = p.j;
     Dir temp_dir = NONE;
     vector<Dir> v;
     bool found = false;
     v.push_back(TOP); v.push_back(BOTTOM); v.push_back(LEFT); v.push_back(RIGHT); v.push_back(TOP); v.push_back(BOTTOM); v.push_back(LEFT); v.push_back(RIGHT);
     random_shuffle(v.begin(), v.end());

     int count = 0; // Si son dels 4 primers tmb comprovarem si hi ha ciutat (cosa prioritaria)
     while (v.size() > 1 and not found)
     {
       int x = i, y = j;
       temp_dir = v.back();
       if (temp_dir == TOP) --x;
       else if (temp_dir == BOTTOM) ++x;
       else if (temp_dir == LEFT) --y;
       else if (temp_dir == BOTTOM) ++y;

       if (count < 4) found = (contingut[x][y] != ENEMIC and mapa[x][y] == CITY);
       else found = (contingut[x][y] != ENEMIC);

       if (not found) v.pop_back();
       ++count;
     }
     next_step = v.back();
   }

   int vida_enemic(Pos p, Dir dir)
   {

     int x = p.i, y = p.j;
     if (dir == TOP) --x;
     else if (dir == BOTTOM) ++x;
     else if (dir == LEFT) --y;
     else if (dir == BOTTOM) ++y;
     if (not pos_ok(x,y)) return 0;
     int enemy_id = cell(x, y).unit_id;
     if (enemy_id == -1) return 0;
     return unit(enemy_id).health;
   }

   int get_adj_city_id(Pos p)
   {
     int x = p.i, y = p.j;
     if(mapa[x-1][y] == CITY) return cell(x-1, y).city_id;
     else if(mapa[x+1][y] == CITY) return cell(x+1, y).city_id;
     else if(mapa[x][y-1] == CITY) return cell(x, y-1).city_id;
     else if(mapa[x][y+1] == CITY) return cell(x, y+1).city_id;
     return -1;
   }

   //////////////////// MOVE UNITATS ////////////////////////////////////

   Dir move_campejar(Unit unitat, Pos& pos_objectiu)
   {
     Dir r = NONE;
     int x = unitat.pos.i, y = unitat.pos.j;
     //cerr << "MOVING CAMP" << endl;
     //cerr << "Unitat " << unitat.id << " " << unitat.pos.i << " " << unitat.pos.j << endl;

     //if (r == NONE and unitat.health < vida_fugir) fugir(unitat.pos, r, pos_objectiu);
     if (r == NONE and is_city(x, y) and unitats_enemigues_ciutat(get_adj_city_id(unitat.pos), true) == 0)
        {
          int temp_dist = distancia_max_bfs;
          distancia_max_bfs = city(cell(x, y).city_id).size();
          r = sub_find_closer(unitat, pos_objectiu, false, false, true);
          distancia_max_bfs = temp_dist;
        }
     if (r == NONE) r = sub_find_closer_enemy(unitat, pos_objectiu, true, true);
     if (r == NONE)
     {
       vector<Dir> v;
       bool found = false;
       v.push_back(TOP); v.push_back(BOTTOM); v.push_back(LEFT); v.push_back(RIGHT);
       random_shuffle(v.begin(), v.end());

       while (v.size() > 0 and not found)
       {
         //cerr << "Checking for mov to " << v.back() << endl;
         found = (future_pos_type(v.back(), x, y) == CITY);
         if (not found) v.pop_back();
       }
       if (found) r = v.back();
     }

     // Program to move the unit around the city and avoid enemies
     return r;
   }

   void calcular_premoviments()
   {
     Unit unitat;
     Dir next_step = NONE;
     Pos pos_objectiu;
     Cell temp_cell;

     for (int i = 0; i < (int)unitats.size(); ++i)
     {
       next_step = pre_moviments[i].second;
       unitat = unit(unitats[i]);
       temp_cell = cell(unitat.pos);

       if (campejar and unitat.damage == 0 and is_city(unitat.pos)) assignar_campejar(unitat);

       //if (unitat.health < vida_fugir and unitats_enemigues_adjacents(unitat.pos) > 1) fugir(unitat.pos, next_step, pos_objectiu);
       if (is_city(unitat.pos) and city_owner(temp_cell.city_id) != me() and next_step == NONE)  next_step = sub_find_closer_enemy(unitat, pos_objectiu, true, true);

       if (campejar and camping(unitat.id) and next_step == NONE) next_step = move_campejar(unitat, pos_objectiu);
       //if (next_step == NONE and unitat.health < vida_fugir) next_step = sub_find_closer(unitat, pos_objectiu);

       if (kamikaze and unitat.damage != 0 and round() < rounds_kamikaze) // Els que estiguin infectats aniran de kamikazes
       {
         if (next_step == NONE)                                     next_step = sub_find_closer_enemy(unitat, pos_objectiu, false, false);
       }

       if (next_step == NONE)
       {
         atacar_enemic_aprop(unitat.pos, next_step, pos_objectiu);

         // Farem que si tenim l'as de perdre millor marxar si podem per dintre la base
         if (next_step != NONE and vida_enemic(unitat.pos, next_step) > 45 and unitat.health < 35)
         {
           next_step = sub_find_closer(unitat, pos_objectiu, false, true, false);
         }
       }

       if (next_step == NONE and not unitat.mask and not unitat.immune)           mascara_aprop(unitat.pos, next_step, pos_objectiu);

       if (next_step == NONE)                                            next_step = sub_find_closer(unitat, pos_objectiu, false, false, false);

       //indicar_casella_ocupada(unitat.pos, next_step);
       if (assignacions) eliminar_assignacio(unitat, pos_objectiu);

       pre_moviments[i] = make_pair(unitat.id, next_step);
       //cerr << "La unitat amb id: " << unitat.id << " es mou a " << next_step << endl;
     }
   }

   void move_unitats()
   {

     obtain_round_data();
     if (campejar and grup_campejar == -1) eliminar_campejar();

     pre_moviments.clear();
     for (int i : unitats) pre_moviments.push_back(make_pair(i, NONE));

     calcular_premoviments();

     if (assignacions)
     {
       set<int> a_reftificar;
       a_reftificar = eliminar_assignacio_insuficient();

       int temp_dist = distancia_max_bfs;
       while (not a_reftificar.empty() and (int)ciutats_bloquejades.size() < nb_cities() and (int)camins_bloquejades.size() < nb_paths())
       {
         for (int i = 0; i < (int)pre_moviments.size(); ++i)
         {
           Unit unitat = unit(unitats[i]);
           Pos pos_objectiu;
           if (in_set(pre_moviments[i].first, a_reftificar))
           {
             pre_moviments[i].second = sub_find_closer(unitat, pos_objectiu, false, false, false);
             if (assignacions) eliminar_assignacio(unitat, pos_objectiu);
           }
           if (assignacions) a_reftificar = eliminar_assignacio_insuficient();
         }
         distancia_max_bfs += 5;
       }

       if (not a_reftificar.empty())
       {
         ciutats_bloquejades.clear();
         for (int i = 0; i < (int)pre_moviments.size(); ++i)
         {
           Unit unitat = unit(unitats[i]);
           Pos pos_objectiu;
           if (in_set(pre_moviments[i].first, a_reftificar))
           {
             pre_moviments[i].second = sub_find_closer(unitat, pos_objectiu, false, false, false);
           }
         }
       }
       distancia_max_bfs = temp_dist;
     }

     for (int i = 0; i < (int)pre_moviments.size(); ++i)
     {
       move(pre_moviments[i].first, pre_moviments[i].second);
     }

   }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play ()
  {
    obtain_round_data();
    //cerr<<"round data obtained" << endl;
    double st = status(me());
    if (st >= 0.8) // Si passem de 80%
    {
      easy_kill = false;
      kamikaze = true;
      assignacions = false;
      campejar = false;
      limit = false;
    }
    if (st >= 0.6) // Si passem de 60%
    {
      easy_kill = false;
      kamikaze = true;
      assignacions = false;
      campejar = false;
      limit = true;
    }
    // if (round() == 199)
    // {
    //   for(auto x : camins_ciutats)
    //   {
    //     cerr << "Ciutat " << x.first << endl;
    //     cerr << "Sortida A " << x.second.first.i << " " << x.second.first.j << endl;
    //     cerr << "Sortida B " << x.second.second.i << " " << x.second.second.j << endl;
    //   }
    // }

    move_unitats();
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
