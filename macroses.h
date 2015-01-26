#ifndef MACROSES_H
#define MACROSES_H
#include <iostream>
#define ASSERT(op) if(!(op)) {std::cerr<<__FILE__<<" ["<<__LINE__<<"] \'"<<#op<<"\' ASSERTION FAILED!\n"; std::terminate();}

#endif // MACROSES_H

