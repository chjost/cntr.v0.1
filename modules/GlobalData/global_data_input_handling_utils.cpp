#include "global_data.h"
#include "global_data_utils.h"

namespace {

// *****************************************************************************
// A helper function to simplify the main part.
template<class T>
inline std::ostream& operator<< (std::ostream& os, const std::vector<T>& v) {
  std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
  return os;
}
// *****************************************************************************
/// @brief Stream insertion operator for slave.
///
/// @param stream The stream into which quark is being inserted.
/// @param q The quark object.
///
/// @return Reference to the ostream.
inline std::ostream& operator<< (std::ostream& stream, const quark& quark) {
  return stream << "\tQUARK type: ****  " << quark.type
      << "  ****\n\t number of random vectors: " << quark.number_of_rnd_vec
      << "\n\t dilution scheme in time: " 
        << quark.dilution_T << quark.number_of_dilution_T 
      << "\n\t dilution scheme in ev space: "
        << quark.dilution_E << quark.number_of_dilution_E
      << "\n\t dilution scheme in Dirac space: " 
        << quark.dilution_D<< quark.number_of_dilution_D 
      << "\n\t path of the perambulator and random vectors:\n\t\t"
         << quark.path <<  "\n";
}

// *****************************************************************************
inline std::array<int, 3> create_3darray_from_string(std::string in) { 

  std::array<int, 3> out;
  std::vector<std::string> tokens;
  // erasing the brakets at the beginning and the end
  in.erase(0,2);
  in.erase(in.end()-1);

  boost::split(tokens, in, boost::is_any_of(","));

  return {{boost::lexical_cast<int>(tokens[0]),
          boost::lexical_cast<int>(tokens[1]),
          boost::lexical_cast<int>(tokens[2]) }};

}
// *****************************************************************************
inline void create_all_momentum_combinations(const int p, 
                                        std::vector<std::array<int, 3> >& out) {
  // creating all momentum combinations possible and needed
  int max_p = p;
  std::vector<std::array<int, 3> > all_p;
  for(int p1 = -max_p; p1 < max_p+1; p1++)
    for(int p2 = -max_p; p2 < max_p+1; p2++)
      for(int p3 = -max_p; p3 < max_p+1; p3++)
        all_p.push_back({{p1, p2, p3}});
  // copying wanted combinations into out array
  for(const auto& all : all_p)
    if(p == all[0]*all[0] + all[1]*all[1] + all[2]*all[2])
      out.push_back(all);

}
// *****************************************************************************
inline void create_mom_array_from_string(std::string in, 
                                        std::vector<std::vector<std::array
                                                   <int, 3> > >& out) {
  // erase the p (first entry)
  in.erase(0,1);
  std::vector<std::string> tokens;
  boost::split(tokens, in, boost::is_any_of(","));
  int p;
  size_t counter = 0;
  out.resize(tokens.size());
  for(const auto& t : tokens){
    p = boost::lexical_cast<int>(t);
    create_all_momentum_combinations(p, out[counter]);
    counter++;
  }

}


} // end of unnamed namespace
/******************************************************************************/
/******************************************************************************/

namespace global_data_utils {

/*! 
 *  @param quark_string Quark as specified in the infile
 *                      quark = @em flavor : @em nb_rnd_vec : T @em diltT_type : 
 *                              @em dilT : E @em dilE_type : @em dilE : 
 *                              D @em dilD_type : @em dilD : @em path
 *  where the following abbreviations where used
 *  - @em flavor     {u,s,c,b} : Quark flavor
 *  - @em nb_rnd_vec       : Number of random vectors
 *  - @em dilT_type  {I,B} : Dilution type in time 
 *  - @em dilT             : Number of dilution blocks in time
 *  - @em dilE_type  {I,B} : Dilution type in eigenvetor space 
 *  - @em dilE             : Number of dilution blocks in eigenvector space 
 *  - @em dilD_type  {I,B} : Dilution type in dirac space 
 *  - @em dilD             : Number of dilution blocks in Dirac space
 *  - @em path             : Path to perambulator
 *  The validity of the values is checked in quark_check()
 *
 *  @returns A quark object constructed with the data obtained from 
 *           @em quark_string
 *
 *  Internally uses boost to split the string and process the parts. 
 */       
quark make_quark (const std::string& quark_string) {
  // Tokenize the string on the ":" delimiter.
  std::vector<std::string> tokens;
  boost::split(tokens, quark_string, boost::is_any_of(":"));

  // If the split did not result in exactly 8 tokens, then the value
  // is formatted wrong.
  if(9 != tokens.size()){
    using boost::program_options::validation_error;
    throw validation_error(validation_error::invalid_option_value,
        "quarks.quark", quark_string);
  }

  // Create a quark from the token values.
  return quark(tokens[0], boost::lexical_cast<int>(tokens[1]), tokens[2],
      boost::lexical_cast<int>(tokens[3]), tokens[4],
      boost::lexical_cast<int>(tokens[5]), tokens[6],
      boost::lexical_cast<int>(tokens[7]), 0, tokens[8]);
}

void quark_check (quark quarks) {

  try{
    if(quarks.type != "u" && quarks.type != "d" && quarks.type != "s"
        && quarks.type != "c"){
      std::cout << "quarks.quark.type must be u, d, s or c" << std::endl;
      exit(0);
    }
    /*! @todo Check that the number of random vectors is greater than the 
     *  largest required diagram
     */
    else if(quarks.number_of_rnd_vec < 1){
      std::cout << "quarks.quark.number_of_rnd_vec must be greater than 0"
          << std::endl;
      exit(0);
    }
    else if(quarks.dilution_T != "TI" && 
            quarks.dilution_T != "TB" &&
            quarks.dilution_T != "TF"){
      std::cout << "quarks.quark.dilution_T must be TI, TB, TF" << std::endl;
      exit(0);
    }
    else if(quarks.number_of_dilution_T < 1){
      std::cout << "quarks.quark.number_of_dilution_T must be greater than 0 "
          "and smaller than the temporal extend" << std::endl;
      exit(0);
    }
    else if(quarks.dilution_E != "EI" && 
            quarks.dilution_E != "EB" &&
            quarks.dilution_E != "EF"){
      std::cout << "quarks.quark.dilution_E must be EI, EB or EF" << std::endl;
      exit(0);
    }
    else if(quarks.number_of_dilution_E < 1){
      std::cout << "quarks.quark.number_of_dilution_E must be greater than 0 "
          "and smaller than number of eigen vectors" << std::endl;
      exit(0);
    }
    else if(quarks.dilution_D != "DI" && 
            quarks.dilution_D != "DB" &&
            quarks.dilution_D != "DF"){
      std::cout << "quarks.quark.dilution_D must be DI, DB or DF" << std::endl;
      exit(0);
    }
    else if(quarks.number_of_dilution_D < 1 || quarks.number_of_dilution_D > 4){
      std::cout << "quarks.quark.number_of_dilution_D must be greater than 0 "
          "and smaller than 5" << std::endl;
      exit(0);
    }
    else std::cout << quarks << std::endl;
  }
  catch(std::exception& e){
    std::cout << e.what() << "\n";
    exit(0);
  }

}

/*****************************************************************************/
/*!
 *  @param operator_string  Operator as specified in the infile:
 *                          A ';'-sperated list with individual operators. The
 *                          individual operators are composed of '.'-seperated 
 *                          parts. E.g. 
 *                          @code
 *                            operator_list = g4.d0.p(0,0,1);g5.d0.p0,1
 *                          @endcode
 *                          Momenta (<em>p</em>) can be specified as 3-momentum or
 *                          by (one or more) scalar number(s). In the latter 
 *                          case all 3-momenta with corresponding absolute 
 *                          value are constructed
 *
 *  @returns An Operator_list object constructed with the data obtained from
 *           @em operator_string
 *
 *  Internally uses boost to split the string and process the parts. 
 */
Operator_list make_operator_list(const std::string& operator_string) {

  Operator_list op_list; // return object

  // Two steps are necessary: 
  // 1. Getting all operators in one list which are separated by ";"
  // 2. Separating the individual operators into its smaller bits, which are 
  //    separated by "."
  // Tokenize the string on the ";" delimiter -> Individual operators
  std::vector<std::string> operator_tokens;
  boost::split(operator_tokens, operator_string, boost::is_any_of(":"));

  // running over opeator tokens and split them further (Step 2):
  for (const auto& op_t : operator_tokens){
    std::vector<std::string> tokens;
    boost::split(tokens, op_t, boost::is_any_of("."));
    std::vector<int> gammas;
    std::array<int, 3> dil_vec;
    std::vector<std::vector<std::array<int, 3> > >mom_vec;
    for (auto str : tokens){
      // getting the gamma structure
      if(str.compare(0,1,"g") == 0)
        gammas.push_back(boost::lexical_cast<int>(str.erase(0,1)));
      // getting the displacement indices
      else if (str.compare(0,1,"d") == 0) {
        if(str.compare(1,1,"0") == 0)
          dil_vec = {{0, 0, 0}};
        else if (str.compare(1,1,"(") == 0)
          dil_vec = create_3darray_from_string(str);
        else {
         std::cout << "Something wrong with the displacement in the operator" \
                      " definition" << std::endl;
         exit(0);
        }
      }
      // getting the momenta
      else if (str.compare(0,1,"p") == 0) {
        if(str.compare(1,1,"(") == 0){
          mom_vec.resize(1);
          mom_vec[0].push_back(create_3darray_from_string(str));
        }
        else 
          create_mom_array_from_string(str, mom_vec);
      }
      // catching wrong entries
      else {
        std::cout << "there is something wrong with the operators" << std::endl;
        exit(0);
      }
    }
    op_list.push_back(Operators(gammas, dil_vec, mom_vec));
  }
  return op_list;
}

} // end of namespace global_data_utils
