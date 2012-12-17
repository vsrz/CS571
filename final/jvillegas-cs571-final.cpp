#include<iostream>
#include<algorithm>
#include<fstream>
#include<vector>

using namespace std;

bool showExpansion = true; 
bool showCost = true;
bool pauseWait = true;

// Display all possible states for final report
bool showAllGameStates = false;

// Seconds to sleep when debugging is enabled
int sleepduration = 0;

// cost analysis
int tcost = 0;

// Numer of moves before stopping (-1 turns it off)
int loops = -1;

class GameState {

private:
	// Variable for each entity, cannibal, missionary, boat
	// They all represent what is on the W side
	int c;
	int m;
	int b;
	bool winner;

public:

	// Initializes a new GameState object
	GameState(int m=3,int c=3,int b=-1,int winner=false) {
		this->c = c;
		this->m = m;
		this->b = b;
		this->winner = this->c == 0 && this->m == 0 && this->b ==1;
	}

	~GameState() {}

	void setGameState(int b, int m, int c) {
		this->b = b;
		this->c = c;
		this->m = m;
	}

	// Since there are only 6 units on the board, and two
	// places they can be, we only need to keep track of
	// what's on one side
	int getCannibalsN() { return(this->c); }
	int getCannibalsS() { return(3-this->c); }
	int getMissionariesN() { return(this->m); }
	int getMissionariesS() { return(3-this->m); }
	int getBoat() { return(this->b); }
	int setCannibals(int c) { this->c += c; }
	int setMissionaries(int m) { this->m += m; }
	int setBoat() { this->b *= -1; }
	
	// Winning state: all entities are on the other side
	bool isWinner() { 
		return(this->c == 0 && this->m == 0 && this->b ==1);
	}
	
	// Provide a way to compare two gamestates
	bool operator==(GameState g) {
		return(
				this->c == g.getCannibalsN()      &&
				this->m == g.getMissionariesN()   &&
				this->b == g.getBoat());

	}

	bool operator!=(GameState g) {
		return(!(*this == g));
	}


	// -1 for boat on the left side, 1 for boat on right
	
	// Is the current state valid?
	bool isValid() {
		
		// North side
		//  - First check if there are cannibals here to be converted
		if(this->getCannibalsN() > 0) {
			// If there are Cannibals here, will they be converted?
			if(this->getMissionariesN() > this->getCannibalsN())
				return(false);
		
		}

		// South side
		//  - First check if there are cannibals here to be converted
		if(this->getCannibalsS() > 0) {
			// If there are Cannibals here, will they be converted?
			if(this->getMissionariesS() > this->getCannibalsS())
				return(false);
		
		}
		return(true);
	}

	// Is the current state a goal state?
	bool isGoal() {
		if(this->getMissionariesS() == 3 && this->getCannibalsS() == 3)
			this->winner = true;
		return(true);
	}

	GameState move(int numM = 0, int numC = 0) {

		// Creates a new instance of the GameState object with the current gamestate
		GameState next(this->m,this->c,this->b);

		// Moves the requested units
		next.setMissionaries(numM*this->b);
		next.setCannibals(numC*this->b);
		next.setBoat();

		return(next);

	}

	// Display the gamestate on screen
	void printState() {
		std::cout<<"#########"<<endl<<"#..";

		for(int i = 0; i < 3; ++i) {
			if(i<this->getMissionariesN()) std::cout << "M";
			else std::cout<<".";
		}

		std::cout << "..#"<<endl<<"#..";

		for(int i = 0; i < 3 ; ++i) {
			if(i<this->getCannibalsN()) std::cout << "C";
			else std::cout<<".";
		}
		std::cout<<"..#"<<endl;
		if(this->b < 0) std::cout << "#~~~B~~~#"<<endl<<"#~~~~~~~#";
		else std::cout<<"#~~~~~~~#"<<endl<<"#~~~B~~~#";
		std::cout<<endl<<"#..";
		for(int i = 0; i < 3 ; ++i) {
			if(i<this->getCannibalsS()) std::cout << "C";
			else std::cout<<".";
		}
		std::cout << "..#"<<endl<<"#..";
		for(int i = 0; i < 3 ; ++i) {
			if(i<this->getMissionariesS()) std::cout << "M";
			else std::cout<<".";
		}
		std::cout<<"..#"<<endl<<"#########"<<endl;

	}


};

class Move {

private:

	int b;
	int m;
	int c;

	vector<Move> generateMoves() {
		vector<Move> m;

		// Boat on the north side, number of m, number of c
		m.push_back(Move(-1,1,0));
		m.push_back(Move(-1,2,0));
		m.push_back(Move(-1,0,1));
		m.push_back(Move(-1,0,2));
		m.push_back(Move(-1,1,1));

		// Boat on the south side, number of m, number of c
		m.push_back(Move(1,1,0));
		m.push_back(Move(1,2,0));
		m.push_back(Move(1,0,1));
		m.push_back(Move(1,0,2));
		m.push_back(Move(1,1,1));

		return(m);
	}

	// Generate all valid moves given the current gamestate
	vector<Move> getMoves(GameState g) {
		vector<Move> m = this->generateMoves();
		vector<Move> r;
		vector<Move>::iterator i;
		if(showExpansion) {
			std::cout<<"== Expansion Analysis =="<<endl;
		}
		for(i = m.begin(); i != m.end(); ++i) {
			GameState n;
			Move nm = *i;
			
			// Throw out any move that doesn't move to boat to the other side
			if(g.getBoat() == nm.getMoveB()) continue;
			
			// Throw out any move that attempts to move a unit that would cause
			// the number of units on a side to be negative
			if(g.getBoat() > 0) {
				if(g.getCannibalsS()-nm.getMoveC() < 0) continue;
				if(g.getMissionariesS()-nm.getMoveM() < 0) continue;
			} else {
				if(g.getCannibalsN()-nm.getMoveC() < 0) continue;
				if(g.getMissionariesN()-nm.getMoveM() < 0) continue;
			}
			
			n = g.move(nm.getMoveM(),nm.getMoveC());
			if(showExpansion) {
				cout<<"Move ["<<nm.getMoveM()<<"] M and ["<<nm.getMoveC()<<"] C "<<(g.getBoat()>0?"North":"South")<<": ";
			}
			if(n.isValid()) {				
				if(showExpansion)cout<<"OK"<<endl;
				r.push_back(nm);
			} else {if(showExpansion)cout<<"Invalid"<<endl; }
		}
		
		/*
		if(showValidMoves) {
			int count = 0;
			cout<<"Show valid moves for this state: "<<endl;
			for(i = r.begin(); i != r.end(); ++i) {
				Move nm = *i;
				count++;
				std::cout<<"Missionaries: "<<nm.getMoveM()<<" Cannibals: "<<nm.getMoveC()<<" Direction: "<<(g.getBoat()<0?"South":"North")<<endl;
			}
			cout<<"There are "<<count<<" valid moves for this state"<<endl;
		}
		*/

		// Return the list of valid moves
		return(r);

	}

public:
	Move(int boat = -1, int m = 0, int c = 0) {
		this->b = boat;
		this->m = m;
		this->c = c;
	}

	~Move() {}

	int getMoveM() { return(m); }
	int getMoveC() { return(c); }
	int getMoveB() { return(b); }

	// Consider the next move based on the current gameState
	Move considerMove(GameState g, vector<GameState> prev) {
	
		// arbitrary high number so it will be set the first time
		// the loop runs
		int bcost = 999;
		Move bestMove;
		
		// Next potential gamestate
		GameState n;
		
		// With the list of valid moves
		vector<Move> validMoves = this->getMoves(g);
		vector<Move>::iterator i;
		int count = 0;
		
		if(showExpansion) {			
			for(i = validMoves.begin(); i != validMoves.end(); ++i)
				count++;
			cout<<"Valid Moves: "<<count<<endl<<"== End of Expansion Analysis =="<<endl;	
		}
		if(showCost)cout<<"== Cost analysis =="<<endl;			
		// Do each one and maintain its cost
		for(i = validMoves.begin(); i != validMoves.end(); ++i) {
			Move m = *i;
			int cost = 0;
			
			// Make the move, then evaluate it
			n = m.makeMove(g);
			
			// Having to move the boat costs 1
			++cost;			
			
			// Increment the cost based on the remaining number of
			// units on the north side of the map
			cost += n.getCannibalsN() + n.getMissionariesN();							
								
			// We're not in the business of going back to where we
			// started, so make sure the new state is not somewhere
			// we've been before.
			vector<GameState>::iterator it;
			it = find(prev.begin(), prev.end(), n);		
			if(it!=prev.end()) {
				cost++;
			}
			
			if(showCost) {				
				cout<<"Cost of moving "<<m.getMoveM()<<" M and ";
				cout<<m.getMoveC()<<" C "<<(n.getBoat()<0?"North":"South");
				cout<<" is h(n): "<<cost<<endl;
				
			}
			
			if(cost < bcost) {
				bcost = cost;				
				bestMove = m;
			}
			
		}
		if(showCost) {
			cout<<"Best move (Cost "<<bcost<<"): ";
			bestMove.printMove();
			cout<<"Cumulative cost g(n): "<<tcost<<endl;
			tcost += bcost;

		}
		return(bestMove);
		
	}
	
	GameState makeMove(GameState n) {
		return(n.move(this->m,this->c));
	}

	// Prints all valid moves given the current gamestate	
	void printAllValidMoves(GameState g) {
		vector<Move> m = this->getMoves(g);
		std::vector<Move>::iterator i;
		for(i = m.begin(); i != m.end(); ++i) {
			Move mv = *i;
			mv.printMove();
		}
	}
	
	void printMove() {
		std::cout<<"Move ["<<this->m<<"] M and ["<<this->c<<"] C to the "<<(this->b > 0?"South":"North")<<endl;
	}


};

void listAllGameStates() {
	// Boat north
	// Missionaries 3
	GameState g;
	g.setGameState(-1,3,3);
	g.printState();
	cout<<endl;
	g.setGameState(-1,3,2);
	g.printState();
	cout<<endl;
	g.setGameState(-1,3,1);
	g.printState();
	cout<<endl;
	g.setGameState(-1,3,0);
	g.printState();
	cout<<endl;
	
	// Missionaries 2
	g.setGameState(-1,2,3);
	g.printState();
	cout<<endl;
	g.setGameState(-1,2,2);
	g.printState();
	cout<<endl;
	g.setGameState(-1,2,1);
	g.printState();
	cout<<endl;
	g.setGameState(-1,2,0);
	g.printState();
	cout<<endl;

	// Missionaries 1
	g.setGameState(-1,1,3);
	g.printState();
	cout<<endl;
	g.setGameState(-1,1,2);
	g.printState();
	cout<<endl;
	g.setGameState(-1,1,1);
	g.printState();
	cout<<endl;
	g.setGameState(-1,1,0);
	g.printState();
	cout<<endl;

	// Missionaries 0
	g.setGameState(-1,0,3);
	g.printState();
	cout<<endl;
	g.setGameState(-1,0,2);
	g.printState();
	cout<<endl;
	g.setGameState(-1,0,1);
	g.printState();
	cout<<endl;
	
	// Boat south
	// Missionaries 3
	g.setGameState(1,3,2);
	g.printState();
	cout<<endl;
	g.setGameState(1,3,1);
	g.printState();
	cout<<endl;
	g.setGameState(1,3,0);
	g.printState();
	cout<<endl;

	// Missionaries 2
	g.setGameState(1,2,3);
	g.printState();
	cout<<endl;
	g.setGameState(1,2,2);
	g.printState();
	cout<<endl;
	g.setGameState(1,2,1);
	g.printState();
	cout<<endl;
	g.setGameState(1,2,0);
	g.printState();
	cout<<endl;

	// Missionaries 1
	g.setGameState(1,1,3);
	g.printState();
	cout<<endl;
	g.setGameState(1,1,2);
	g.printState();
	cout<<endl;
	g.setGameState(1,1,1);
	g.printState();
	cout<<endl;
	g.setGameState(1,1,0);
	g.printState();
	cout<<endl;

	// Missionaries 0
	g.setGameState(1,0,3);
	g.printState();
	cout<<endl;
	g.setGameState(1,0,2);
	g.printState();
	cout<<endl;
	g.setGameState(1,0,1);
	g.printState();
	cout<<endl;
	g.setGameState(1,0,0);
	g.printState();
	cout<<endl;
	
	
}

int main(int argc, char* argv[]) {
	
	// if you want to just show the game boards
	if(showAllGameStates) {
		listAllGameStates();
		return(0);
	}
	
	// holds the list of gamestates for giving solution
	std::vector<GameState> gs;
	std::vector<Move> movelist;
	Move moves;

	// Initial State
	GameState state;
	gs.push_back(state);	
	
	std::cout << "CS571 Final Project by Jeremy Villegas\r\n";
	std::cout << "======================================\r\n\r\n";

	// Display the initial state
	if(showExpansion || showCost) {
		for(std::vector<GameState>::iterator i = gs.begin(); i != gs.end(); ++i) {
			GameState g = *i;
			g.printState();
		}
	}

	int count = 0;
	// Figure out the next move until the goal state is reached
	while(!state.isWinner() and loops-- != 0) {
		Move move;
		// Find the next state, considering the current state
		// and the history of the previous moves
		moves = move.considerMove(state,gs);
		if(showCost)cout<<"== End of Cost Analysis =="<<endl;			
		
		//moves.printMove();
		movelist.push_back(moves);

		// Make the preferred move
		state = moves.makeMove(state);


		// Append the new state to the state history vector
		gs.push_back(state); 
		++count;

		if(pauseWait) {
			char t;

			cout << "End of iteration "<<count<<endl<<"Press N for next, F to finish, or E for exit: ";
			cin  >> t;
			if(t=='e' || t=='E')return(0);
			if(t=='f' || t=='F')pauseWait=false;
		}
		
		if(showExpansion || showCost) {			
			sleep(sleepduration);
			cout<<endl<<"================================="<<endl;
			state.printState();
		}
	}
	
	std::cout<<"Work complete in "<<count<<" moves!"<<endl;
	for(std::vector<Move>::iterator i = movelist.begin(); i != movelist.end(); ++i) {
		Move imove= *i;
		imove.printMove();
	}
	if(showCost) {
		cout<<"Total cost f(n): "<<tcost<<endl;
	}
	
	return(0);
}
