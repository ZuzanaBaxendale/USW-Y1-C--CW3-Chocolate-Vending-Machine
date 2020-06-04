

#include <iostream>
#include <vector>
using namespace std;

enum state { Out_Of_Chocolate, No_Credit, Has_Credit, Dispenses_Chocolate, Maintenance_Mode };

class StateContext;

class State
{
protected:
	StateContext* CurrentContext;
public:
	State(StateContext* Context) { CurrentContext = Context; }
	virtual ~State(void) {}
};

class StateContext
{
protected:
	State* CurrentState = nullptr;
	int stateIndex = 0;
	vector<State*> availableStates;
public:
	virtual ~StateContext(void);
	virtual void setState(state newState);
	virtual int getStateIndex(void);
};

StateContext::~StateContext(void)
{
	for (int i = 0; i < this->availableStates.size(); i++) delete this->availableStates[i];
	this->availableStates.clear();
}

void StateContext::setState(state newState)
{
	this->CurrentState = availableStates[newState];
	this->stateIndex = newState;
}

int StateContext::getStateIndex(void)
{
	return this->stateIndex;
}

class Transition
{
public:
	virtual bool insertMoney(int) { cout << "Error!" << endl; return false; }
	virtual bool makeSelection(int) { cout << "Error!" << endl; return false; }
	virtual bool moneyRejected(void) { cout << "Error!" << endl; return false; }
	virtual bool addChocolate(int) { cout << "Error!" << endl; return false; }
	virtual bool dispense(void) { cout << "Error!" << endl; return false; }
	virtual bool enterPin(int pin) { cout << "Error!" << endl; return false; }
	virtual bool exit(void) { cout << "Error!" << endl; return false; }
};

class ChocoState : public State, public Transition
{
public:
	ChocoState(StateContext* Context) : State(Context) {}
};

class OutOfChocolate : public ChocoState
{
public:
	OutOfChocolate(StateContext* Context) : ChocoState(Context) {}
	bool enterPin(int pin);
	bool moneyRejected(void);
	bool addChocolate(int number);
};

class NoCredit : public ChocoState
{
public:
	NoCredit(StateContext* Context) : ChocoState(Context) {}
	bool insertMoney(int credit);
	bool enterPin(int pin);
};

class HasCredit : public ChocoState
{
public:
	HasCredit(StateContext* Context) : ChocoState(Context) {}
	bool insertMoney(int credit);
	bool makeSelection(int option);
	bool moneyRejected(void);
};

class DispensesChocolate : public ChocoState
{
public:
	DispensesChocolate(StateContext* Context) : ChocoState(Context) {}
	bool dispense(void);
};

class MaintenanceMode : public ChocoState
{
public:
	MaintenanceMode(StateContext* Context) : ChocoState(Context) {}
	bool exit(void);
	bool addChocolate(int number);
};

class Chocolate_Dispenser : public StateContext, public Transition
{
	friend class OutOfChocolate;
	friend class NoCredit;
	friend class HasCredit;
	friend class DispensesChocolate;
	friend class MaintenanceMode;
private:
	int inventory = 0; //number of chocolate
	int credit = 0; //a measure of the number of bars that can be purchased and not money
	int pin = 54321; //secret pin for maintenance mode - *DO NOT CHANGE*
public:
	Chocolate_Dispenser(void);
	bool insertMoney(int credit);
	bool makeSelection(int option);
	bool moneyRejected(void);
	bool addChocolate(int number);
	bool dispense(void);
	bool enterPin(int pin);
	bool exit(void);
};


Chocolate_Dispenser::Chocolate_Dispenser(void)
{
	this->availableStates.push_back(new OutOfChocolate(this));
	this->availableStates.push_back(new NoCredit(this));
	this->availableStates.push_back(new HasCredit(this));
	this->availableStates.push_back(new DispensesChocolate(this));
	this->availableStates.push_back(new MaintenanceMode(this));

	this->setState(Out_Of_Chocolate);
}

bool Chocolate_Dispenser::insertMoney(int credit)
{
	return ((ChocoState*)CurrentState)->insertMoney(credit);
}

bool Chocolate_Dispenser::makeSelection(int option)
{
	return ((ChocoState*)CurrentState)->makeSelection(option);
}

bool Chocolate_Dispenser::moneyRejected(void)
{
	return ((ChocoState*)CurrentState)->moneyRejected();
}

bool Chocolate_Dispenser::addChocolate(int number)
{
	return ((ChocoState*)CurrentState)->addChocolate(number);
}

bool Chocolate_Dispenser::dispense(void)
{
	return ((ChocoState*)CurrentState)->dispense();
}

bool Chocolate_Dispenser::enterPin(int pin)
{
	return ((ChocoState*)CurrentState)->enterPin(pin);
}

bool Chocolate_Dispenser::exit(void)
{
	return ((ChocoState*)CurrentState)->exit();
}

bool OutOfChocolate::addChocolate(int number)
{
((Chocolate_Dispenser*)CurrentContext)->inventory += number;
cout << "Adding chocolate... Inventory = " << ((Chocolate_Dispenser*)CurrentContext)->inventory << endl;
CurrentContext->setState(No_Credit);
return true;
}

bool OutOfChocolate::enterPin(int pin)
{
	if (pin != 54321)
	{
		CurrentContext->setState(Out_Of_Chocolate);
		cout << "Pin incorrect!" << endl;
		return false;
	}
	CurrentContext->setState(Maintenance_Mode);
	cout << "Pin correct..." << endl;
	return true;
}

bool OutOfChocolate::moneyRejected(void)
{
	cout << "Rejecting money....." << endl;
	((Chocolate_Dispenser*)CurrentContext)->credit = 0;
	CurrentContext->setState(Out_Of_Chocolate);
	return true;
}

bool NoCredit::insertMoney(int credit)
{
	((Chocolate_Dispenser*)CurrentContext)->credit += credit;
	cout << "Adding credit... Credit = " << ((Chocolate_Dispenser*)CurrentContext)->credit << endl;
	CurrentContext->setState(Has_Credit);
	return true;
}

bool NoCredit::enterPin(int pin)
{
	CurrentContext->setState(No_Credit);
	return false;
}

bool HasCredit::insertMoney(int credit)
{
	((Chocolate_Dispenser*)CurrentContext)->credit += credit;
	cout << "Adding credit--- Credit = " << ((Chocolate_Dispenser*)CurrentContext)->credit << endl;
	CurrentContext->setState(Has_Credit);
	return true;
}

bool HasCredit::makeSelection(int option)
{
	//in this simple example, option = number of bars, but coudle be used to reperesent a menu choice
	cout << "You have selected " << option << " bar(s) of chocolate" << endl;

	if (((Chocolate_Dispenser*)CurrentContext)->inventory < option)
	{
		cout << "Error: you have selected more chocolate than the machine contains" << endl;
		return false;
	}

	if (((Chocolate_Dispenser*)CurrentContext)->credit < option)
	{
		cout << "Error: you don't have enough money for that selection" << endl;
		return false;
	}

	cout << "Credit and inventory is sufficient for your selection" << endl;

	((Chocolate_Dispenser*)CurrentContext)->inventory -= option; //deduct inventory
	((Chocolate_Dispenser*)CurrentContext)->credit -= option; //deduct inventory

	CurrentContext->setState(Dispenses_Chocolate);

	return true;
}

bool HasCredit::moneyRejected(void)
{
	cout << "Rejecting money....." << endl;
	((Chocolate_Dispenser*)CurrentContext)->credit = 0;
	CurrentContext->setState(No_Credit);
	return true;
}

bool DispensesChocolate::dispense(void)
{
	cout << "Dispensing..." << endl;
	cout << "Inventory = " << ((Chocolate_Dispenser*)CurrentContext)->inventory << endl;
	cout << "Credit = " << ((Chocolate_Dispenser*)CurrentContext)->credit << endl;

	if (((Chocolate_Dispenser*)CurrentContext)->inventory == 0) CurrentContext->setState(Out_Of_Chocolate);
	else if (((Chocolate_Dispenser*)CurrentContext)->credit == 0) CurrentContext->setState(No_Credit);
	else CurrentContext->setState(Has_Credit);
	return true;
}

bool MaintenanceMode::exit(void)
{
	if (((Chocolate_Dispenser*)CurrentContext)->inventory == 0)
	{
		CurrentContext->setState(Out_Of_Chocolate);
	}
	if (((Chocolate_Dispenser*)CurrentContext)->inventory > 0 & ((Chocolate_Dispenser*)CurrentContext)->credit == 0)
	{
		CurrentContext->setState(No_Credit);
	}
	else
	{
		CurrentContext->setState(Has_Credit);
	}

	return true;
}

bool MaintenanceMode::addChocolate(int number)
{
	cout << "Adding " << number << " bars of chocolate..." << endl;
	((Chocolate_Dispenser*)CurrentContext)->inventory += number;
	return true;
}

int main(void)
{
	Chocolate_Dispenser MyDispenser;
	int pinAttempt;
	int money;
	int option;
	int selection;
	bool exit = false;
	while (exit == false)
	{
		cout << "====================================" << endl;
		cout << "CHOCOLATE VENDING MACHINE MAIN MENU:" << endl;
		cout << "" << endl;
		cout << "Access maintenence mode (1)" << endl;
		cout << "Insert money (2)" << endl;
		cout << "Make selection (3)" << endl;
		cout << "Exit (4)" << endl;
		cout << "====================================" << endl;
		cin >> selection;
		switch (selection)
		{
		case 1: {
			cout << "Please enter the maintanence pin:" << endl;
			cin >> pinAttempt;

			bool pinIsTrue = false;

			pinIsTrue = MyDispenser.enterPin(pinAttempt);

			if (pinIsTrue == true)
			{
				bool exitMaint = false;
				int maintSelection;

				while (exitMaint == false)
				{
					cout << "====================================" << endl;
					cout << "MAINTANENCE MODE MENU:" << endl;
					cout << "" << endl;
					cout << "Add chocolate (1)" << endl;
					cout << "Exit maintanence mode (2)" << endl;
					cout << "====================================" << endl;
					cin >> maintSelection;

					switch (maintSelection)
					{
					case 1: {
						int numChoc = 0;
						cout << "Enter number of chocolate bars to be added:" << endl;
						cin >> numChoc;
						MyDispenser.addChocolate(numChoc);
					}
							break;
					case 2: {
						MyDispenser.exit();
						exitMaint = true;
					}
							break;
					}

				}
			}


		}

				break;
		case 2: { 		
			cout << "Please enter money:" << endl;
			cin >> money;
			MyDispenser.insertMoney(money);
		}
			   break;
		case 3: {
			cout << "Please select an option:" << endl;
			cin >> option;

			bool DispensePossible = MyDispenser.makeSelection(option);

			if (DispensePossible == true)
			{
				MyDispenser.dispense();
			}
			
		
		}
			   break;
		case 4: {exit = true; }
			   break;
		}   

	}

	return 0;
}
