#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/**
 * An account is a structure that represents a bank account.
 * @property {int} balance - The amount of money in the account.
 */
typedef struct __account
{
    int balance;
} account;

/**
 * The type of transaction.
 */
typedef enum __transaction_t
{
    Withdraw,
    Deposit,
    Unknown
} transaction_t;

/**
 * The status of the transaction.
 */
typedef enum __transaction_status
{
    Successful,
    Failed
} transaction_status;

/**
 * Which account actor's thread is currently being executed
 */
typedef enum __actor
{
    Wife,
    Husband
} actor;

/**
 * Struct representing the thread function's arguments
 */
typedef struct __threadArgs
{
    FILE *f;
    account *acc;
    actor user;
} threadArgs;

/**
 * A transaction is a structure that contains a transaction type and an amount.
 * @property {transaction_t} transactionType - The type of transaction.
 * @property {int} amount - The amount of the transaction.
 */
typedef struct __transaction
{
    transaction_t transactionType;
    int amount;
} transaction;

/**
 * A transactionQueue is a struct that contains a long and a pointer to a transaction.
 * @property {long} size - The number of transactions in the queue.
 * @property {transaction} transactions - an array of transactions
 */
typedef struct __transactionQueue
{
    long size;
    transaction *transactions;

} transactionQueue;

/**
 * ExecuteTransactionsStruct is a struct that contains a pointer to an account and a pointer to a
 * transactionQueue.
 * @property {account} transactingAccount - The account that is currently being transacted.
 * @property {transactionQueue} currentTransactionQueue - A queue of transactions that are to be
 * executed.
 */
typedef struct __executeTransactionsStruct
{
    account *transactingAccount;
    transactionQueue *currentTransactionQueue;

} executeTransactionsStruct;

// Declaring the needed functions
void *parseTransactions(FILE *file);
transaction createTransaction(char *transactionString);
void executeTransactions(void *voidExecuteTransactionsStruct, actor acountActor);
void processTransaction(account *transactingAccount, transaction *currentTransaction, actor acountActor);
int deposit(void *account, int amount);
int withdraw(void *account, int amount);
void validateFileNum(int argc);
void *startThread(void *arg);
void *createExecuteTransactionsStruct(account *acc, transactionQueue *tq);
void *createThreadArgStruct(FILE *f, account *acc, actor accountActor);

// Semaphore for Locks
pthread_mutex_t mutex;

int main(int argc, char *argv[])
{

    // checking that arguments are less than 4
    validateFileNum(argc);

    FILE *husband_file, *wife_file;
    int balance = atoi(argv[1]);

    // checking if files can be accessed
    husband_file = fopen(argv[2], "r");
    wife_file = fopen(argv[3], "r");
    if (husband_file == NULL || wife_file == NULL)
    {
        puts("bank: cannot open file");
        exit(1);
    }

    // Enums for checking users
    actor hactor = Husband;
    actor wactor = Wife;

    // // creating an account from the amount specified as an argument
    account theirAccount = {balance};

    // // Major step 1
    // // Print the opening account balance
    // //   in the format specified
    printf("Opening balance: %d\n", theirAccount.balance);

    // Major step 2
    // Read and process transactions in separate threads
    // Create thread function arguments
    void *threadArgStructH = createThreadArgStruct(husband_file, &theirAccount, hactor);
    void *threadArgStructW = createThreadArgStruct(wife_file, &theirAccount, wactor);

    pthread_t husband_thread, wife_thread;
    pthread_mutex_init(&mutex, NULL);
    if (pthread_create(&husband_thread, NULL, &startThread, threadArgStructH) != 0)
    {
        perror("failure to create thread");
    }
    if (pthread_create(&wife_thread, NULL, &startThread, threadArgStructW) != 0)
    {
        perror("failure to create thread");
    }

    if (pthread_join(husband_thread, NULL) != 0)
    {
        perror("Failed to join thread");
    }
    if (pthread_join(wife_thread, NULL) != 0)
    {
        perror("Failed to join thread");
    }

    // Major step 4
    // Print the closing account balance
    //   in the format specified
    pthread_mutex_destroy(&mutex);
    printf("Closing balance: %d\n", theirAccount.balance);

    fclose(husband_file);
    fclose(wife_file);
    return 0;
}

/**
 * It reads a file, and returns a struct containing the number of transactions in the file, and an
 * array of transactions
 *
 * Args:
 *   voidFileName (void): THe name of the file.
 *
 * Returns:
 *   A void pointer to a transactionQueue struct.
 */
void *parseTransactions(FILE *file)
{
    char *line = NULL; // define a variable to store a line
    size_t len = 0;    // variable to store line

    // information for the transactionQueue
    long numTransactions = 0;

    // getting the number of transactions in the queue
    while (!feof(file))
    {
        ssize_t n = getline(&line, &len, file);
        if (n == 1)
        {
            continue;
        }
        numTransactions++;
    }

    transaction *transactions = (transaction *)calloc(numTransactions, sizeof(transaction));
    if (transactions == NULL)
    {
        printf("Memory not allocated.\n");
        exit(0);
    }
    long numTransactionTmp = numTransactions; // temp store for the number of transactions for creating transaction queues

    rewind(file);        // taking the file pointer back to the beginning of the file
    numTransactions = 0; // reset numTransactions
    ssize_t n;
    while ((n = getline(&line, &len, file)) > 0)
    {
        if (!strlen(line))
        {
            continue;
        }

        transaction currentTransaction = createTransaction(line);
        transactions[numTransactions] = currentTransaction;
        numTransactions++;
    }

    transactionQueue *tqPtr = calloc(1, sizeof(transactionQueue)); // remember to free this memory
    *tqPtr = (transactionQueue) {.size = numTransactionTmp, .transactions = transactions};
    return (void *)tqPtr; // take note of this. might be wrong (the casting)
}

/**
 * Create a transaction struct/object from a 'string'
 *
 * Args:
 *   transactionRecord (char): A string containing the transaction type and amount.
 *
 * Returns:
 *   A transaction struct.
 */
transaction createTransaction(char *transactionString)
{
    char *tokenArr[2];
    int i = 0;
    transaction_t type;

    char *token = strtok(transactionString, " ");
    while (token != NULL)
    {
        tokenArr[i] = token;
        i++;
        token = strtok(NULL, " ");
    }

    if (strncmp("deposit", tokenArr[0], 7) == 0)
    {
        type = Deposit;
    }
    else if (strncmp("withdraw", tokenArr[0], 8) == 0)
    {
        type = Withdraw;
    }
    else
    {
        type = Unknown;
    }

    transaction tr = {.transactionType = type, .amount = atoi(tokenArr[1])};
    return tr;
}

/**
 * Process a queue of transactions
 *
 * Args:
 *   voidExecuteTransactionsStruct (void): a void pointer to the executeTransactionsStruct struct
 */
void executeTransactions(void *voidExecuteTransactionsStruct, actor accountActor)
{
    executeTransactionsStruct *ets = voidExecuteTransactionsStruct;

    transactionQueue *tq = ets->currentTransactionQueue;         // transaction queue from the struct
    long queueSize = tq->size;                                   // the queue size
    transaction *transactions = (transaction *)tq->transactions; // the transaction array from the transactionQueue

    account *currentAccount = ets->transactingAccount;

    for (int i = 0; i < queueSize; i++)
    {
        processTransaction(currentAccount, &transactions[i], accountActor);
    }
}

/**
 * "This function takes a pointer to an account and a pointer to a transaction,
 * and then processes the transaction on the account."
 *
 * Args:
 *   transactingAccount (account): This is a pointer to account that is being transacted on.
 *   currentTransaction (transaction): a pointer to a transaction struct
 */
void processTransaction(account *transactingAccount, transaction *currentTransaction, actor accountActor)
{
    char *user = (accountActor == Wife) ? "Wife" : "Husband";
    pthread_mutex_lock(&mutex);
    if (currentTransaction->transactionType == Deposit)
    {
        int res = deposit(transactingAccount, currentTransaction->amount);
        if (res == Successful)
        {
            printf("Deposit: %d, User: %s, Account balance after: %d\n", currentTransaction->amount, user, transactingAccount->balance);
        }
        else
        { // probably not useful
            printf("Deposit: %d, User: %s, Transaction declined\n", currentTransaction->amount, user);
        }
    }
    else if (currentTransaction->transactionType == Withdraw)
    {
        int res = withdraw(transactingAccount, currentTransaction->amount);
        if (res == Successful)
        {
            printf("Withdraw: %d, User: %s, Account balance after: %d\n", currentTransaction->amount, user, transactingAccount->balance);
        }
        else
        {
            printf("Deposit: %d, User: %s, Transaction declined\n", currentTransaction->amount, user);
        }
    } // something might go here for when transaction type is unknown
    pthread_mutex_unlock(&mutex);
}

/**
 * It withdraws money from the provided account
 *
 * Args:
 *   account (void *): The account to be used
 *   amount  (int): the amount to be withdrawn
 */
int withdraw(void *transactingAccount, int amount)
{
    account *acc = transactingAccount;
    transaction_status ts = Successful;

    if (acc->balance < 0 || acc->balance < amount)
    {
        ts = Failed;
        return ts;
    }
    acc->balance -= amount;
    return ts;
}

/**
 * Deposit an amount into provided account
 *
 * Args:
 *   account (void *): The account to be used
 *   amount  (int): The amount to be deposited
 */
int deposit(void *transactingAccount, int amount)
{
    account *acc = transactingAccount;
    acc->balance += amount;

    transaction_status ts = Successful;
    return ts;
}

/**
 * Validates the amount of arguments passed to the program
 *
 * Args:
 *  argc (int): The number of arguments passed
 */
void validateFileNum(int argc)
{
    if (argc < 4 || argc > 4)
    {
        puts("bank: amount file1 file2");
        exit(1);
    }
}

void *createExecuteTransactionsStruct(account *acc, transactionQueue *tq)
{ // might update return type to a pointer
    executeTransactionsStruct *ets = calloc(1, sizeof(executeTransactionsStruct));
    ets->transactingAccount = acc;
    ets->currentTransactionQueue = tq;
    return ets;
}

void *createThreadArgStruct(FILE *f, account *acc, actor accountActor)
{
    threadArgs *threadArgStruct = calloc(1, sizeof(threadArgs));
    threadArgStruct->f = f;
    threadArgStruct->acc = acc;
    threadArgStruct->user = accountActor;
    return (void *)threadArgStruct; // take note also
}

/**
 * The starting point of the thread that executes other functionshu
 */
void *startThread(void *arg)
{
    threadArgs *ta = arg;
    transactionQueue *tq = parseTransactions(ta->f);
    executeTransactionsStruct *ets = createExecuteTransactionsStruct(ta->acc, tq);
    executeTransactions(ets, ta->user);

    free(ets);

    return 0;
}