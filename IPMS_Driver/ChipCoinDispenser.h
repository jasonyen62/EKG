/* Chip Coin Dispenser Function */
/* ¦NÂE   */

#ifndef ____CHIPCOINDISPENSER____
#define ____CHIPCOINDISPENSER____

#include "rs232.h"
class ChipCoinDispenser
{
public:
	ChipCoinDispenser(void);
	~ChipCoinDispenser(void);
	
	bool Initial(enum COMPORT port);
	bool Issue(short id);
	void Reset(short id);
	bool CheckCoinLow(short id);
		
private:
	RS232 comport;
};

#endif
