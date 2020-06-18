#include "instance_abstract.h"
#include "bound_abstract.h"
#include "bound_fsp_strong.h"

int * bound_fsp_strong::tempsJob    = NULL; // [nbMachines][nbJob]
int * bound_fsp_strong::tabJohnson  = NULL; // [somme][nbJob]
int * bound_fsp_strong::tempsLag    = NULL; // [somme][nbJob]
int * bound_fsp_strong::minTempsDep = NULL; // [somme][nbJob]
int * bound_fsp_strong::minTempsArr = NULL; // [somme][nbJob]
int * bound_fsp_strong::machine     = NULL;

void
bound_fsp_strong::init()
{
    initialiserVar();

    flag   = (int *) malloc(nbJob * sizeof(int));
    front  = (int *) malloc(nbMachines * sizeof(int));
    back   = (int *) malloc(nbMachines * sizeof(int));
    remain = (int *) malloc(nbMachines * sizeof(int));
}

void
bound_fsp_strong::configureBound(const int _branchingMode, const int _earlyExit, const int _machinePairs)
{
    branchingMode = _branchingMode;
    earlyExit     = _earlyExit;
    machinePairs  = _machinePairs;
}

void
bound_fsp_strong::initialiserVar()
{
    remplirNbJobNbMachines();
    initSomme();
    allouerMemoire();
    remplirTempsJob();
    remplirTempsArriverDepart();
    remplirMachine();
    remplirLag();
    remplirTabJohnson();

    for (int i = 0; i < somme; i++) {
        rewards[i] = 0;
        countMachinePairs[i] = 0;
        machinePairOrder[i]  = i;
    }
}

inline
void
bound_fsp_strong::initCmax(int * tmp, int * ma, int ind)
{
    ma[0] = machine[ind];
    ma[1] = machine[somme + ind];

    int m0 = ma[0];
    int m1 = ma[1];

    tmp[0] = front[m0];
    tmp[1] = front[m1];
}

inline
void
bound_fsp_strong::cmaxFin(int * tmp, int * ma)
{
    if (tmp[1] + back[ma[1]] > tmp[0] + back[ma[0]])
        tmp[1] = tmp[1] + back[ma[1]];
    else
        tmp[1] = tmp[0] + back[ma[0]];
}

inline
void
bound_fsp_strong::heuristiqueCmax(int * tmp, int * ma, int ind)
{
    int jobCour;
    int tmp0 = tmp[0];
    int tmp1 = tmp[1];
    int ma0  = ma[0];
    int ma1  = ma[1];

    for (int j = 0; j < nbJob; j++) {
        // ind=row in tabJohnson corresponding to ma0 and ma1
        // tabJohnson contains jobs sorted according to johnson's rule
        jobCour = tabJohnson[ind * nbJob + j];
        // j-loop is on unscheduled jobs... (==0 if jobCour is unscheduled)
        if (flag[jobCour] == 0) {
            // add jobCour to ma0 and ma1
            tmp0 += tempsJob[ma0 * nbJob + jobCour];
            if (tmp1 > tmp0 + tempsLag[ind * nbJob + jobCour])
                tmp1 += tempsJob[ma1 * nbJob + jobCour];
            else
                tmp1 = tmp0 + tempsLag[ind * nbJob + jobCour] + tempsJob[ma1 * nbJob + jobCour];
        }
    }
    tmp[0] = tmp0;
    tmp[1] = tmp1;
}

int
bound_fsp_strong::borneInfMakespan(int * valBorneInf, int minCmax)
{
    int moinsBon = 0;
    int ma[2];  /*Contient les rang des deux machines considere.*/
    int tmp[2]; /*Contient les temps sur les machines considere*/

    int i, j, l;
    int bestind = 0;

    // sort machine-pairs by success-count (if earlyExit enabled)
    // at most one swap...
    if (earlyExit) {
        i = 1, j = 2;
        while (i < somme) {
            if (countMachinePairs[machinePairOrder[i - 1]] < countMachinePairs[machinePairOrder[i]]) {
                myswap(&machinePairOrder[i - 1], &machinePairOrder[i]);
                if ((--i)) continue;
            }
            i = j++;
        }
    }

    // for all machine-pairs : O(m^2) m*(m-1)/2
    for (l = 0; l < somme; l++) {
        // start by most successful machine-pair....only useful if early exit allowed
        i = machinePairOrder[l];
        // add min start times and get ma[0],ma[1] from index i
        initCmax(tmp, ma, i);

        //(1,m),(2,m),...,(m-1,m)
        if (machinePairs == 1 && ma[1] < nbMachines - 1) continue;
        //(1,2),(2,3),...,(m-1,m)
        if (machinePairs == 2 && ma[1] != ma[0] + 1) continue;

        // compute cost for johnson sequence //O(n)
        heuristiqueCmax(tmp, ma, i);
        // complete bound with scheduled cost
        cmaxFin(tmp, ma);

        // take max
        if (tmp[1] > moinsBon) {
            //best index
            bestind  = i;
            //update max
            moinsBon = tmp[1];
        }

        // early exit from johnson (if lb > best)
        if (moinsBon > minCmax && minCmax != -1) {
            valBorneInf[0] = moinsBon;
            break;
        }
    }

    //++ successful machine pair
    countMachinePairs[bestind]++;

    valBorneInf[0] = moinsBon;
    return bestind;
} // borneInfMakespan

int
bound_fsp_strong::borneInfLearn(int * valBorneInf, int UB, bool earlyExit)
{
    //reset periodically...
    if (nbbounds > 100 * 2 * nbJob) {
        nbbounds = 0;
        for (int k = 0; k < somme; k++) {
            rewards[k] = 0;///=100;
        }
    }

    int maxLB = 0;
    int ma[2];  /*Contient les rang des deux machines considere.*/
    int tmp[2]; /*Contient les temps sur les machines considere*/

    int i, j, l;
    int bestind = 0;

    i = 1, j = 2;
    while (i < somme) {
        if (rewards[machinePairOrder[i - 1]] < rewards[machinePairOrder[i]]) {
            myswap(&machinePairOrder[i - 1], &machinePairOrder[i]);
            if ((--i)) continue;
        }
        i = j++;
    }

    //restrict to best nbMachines
    int nbPairs = nbMachines;
    //learn...
    if (nbbounds < 2 * nbJob) nbPairs = somme;
    nbbounds++;

    for (l = 0; l < nbPairs; l++) {
        // start by most successful machine-pair....only useful if early exit allowed
        i = machinePairOrder[l];

        initCmax(tmp, ma, i);// add min start times
        heuristiqueCmax(tmp, ma, i);// compute johnson sequence //O(n)
        cmaxFin(tmp, ma);

        if (tmp[1] > maxLB) {
            bestind = i;
            maxLB   = tmp[1];
        }
        if (earlyExit && (maxLB > UB) && (nbPairs < somme)) {
            break;
        }
    }

    rewards[bestind]++;

    valBorneInf[0] = maxLB;
    return bestind;
} // bound_fsp_strong::borneInfLearn

// =====================================
//      INITIALIZATION
// =====================================
void
bound_fsp_strong::remplirNbJobNbMachines()
{
    (instance->data)->seekg(0);
    (instance->data)->clear();
    *(instance->data) >> nbJob;
    *(instance->data) >> nbMachines;
}

void
bound_fsp_strong::remplirTempsJob()
{
    for (int j = 0; j < nbMachines; j++)
        for (int i = 0; i < nbJob; i++)
            *(instance->data) >> tempsJob[j * nbJob + i];
}

void
bound_fsp_strong::initSomme()
{
    somme = 0;
    for (int i = 1; i < nbMachines; i++)
        somme += i;
}

void
bound_fsp_strong::allouerMemoire()
{
    // static members
    if (!tempsJob) posix_memalign((void **) &tempsJob, 64, nbJob * nbMachines * sizeof(int));
    if (!tempsLag) posix_memalign((void **) &tempsLag, 64, nbJob * somme * sizeof(int));
    if (!tabJohnson) posix_memalign((void **) &tabJohnson, 64, nbJob * somme * sizeof(int));
    if (!machine) posix_memalign((void **) &machine, 64, 2 * somme * sizeof(int));
    if (!minTempsArr) posix_memalign((void **) &minTempsArr, 64, nbMachines * sizeof(int));
    if (!minTempsDep) posix_memalign((void **) &minTempsDep, 64, nbMachines * sizeof(int));

    rewards = (int *) malloc(somme * sizeof(int));
    countMachinePairs = (int *) malloc(somme * sizeof(int));
    machinePairOrder  = (int *) malloc(somme * sizeof(int));
}

void
bound_fsp_strong::remplirLag()
{
    int m1, m2;

    // for all jobs and all machine-pairs
    for (int i = 0; i < somme; i++) {
        m1 = machine[i];
        m2 = machine[somme + i];
        for (int j = 0; j < nbJob; j++) {
            tempsLag[i * nbJob + j] = 0;
            // term q_iuv in Lageweg'78
            for (int k = m1 + 1; k < m2; k++)
                tempsLag[i * nbJob + j] += tempsJob[k * nbJob + j];
        }
    }
}

void
bound_fsp_strong::remplirMachine()
{
    int cmpt = 0;

    // [0 0 0 ...  0  1 1 1 ... ... M-3 M-3 M-2 ]
    // [1 2 3 ... M-1 2 3 4 ... ... M-2 M-1 M-1 ]
    for (int i = 0; i < (nbMachines - 1); i++) {
        for (int j = i + 1; j < nbMachines; j++) {
            machine[cmpt]         = i;
            machine[somme + cmpt] = j;
            // printf("%d,%d ",machine[cmpt],machine[somme_pad + cmpt]);
            cmpt++;
        }
    }
}

void
bound_fsp_strong::remplirTempsArriverDepart()
{
    for (int k = 0; k < nbMachines; k++) {
        minTempsDep[k] = 9999999;
    }
    minTempsDep[nbMachines - 1] = 0;
    int * tmp = new int[nbMachines];

    for (int i = 0; i < nbJob; i++) {
        for (int k = nbMachines - 1; k >= 0; k--) {
            tmp[k] = 0;
        }
        tmp[nbMachines - 1] += tempsJob[(nbMachines - 1) * nbJob + i];// ptm[(nbMachines-1) * nbJob + job];
        for (int k = nbMachines - 2; k >= 0; k--) {
            tmp[k] = tmp[k + 1] + tempsJob[k * nbJob + i];
        }
        for (int k = nbMachines - 2; k >= 0; k--) {
            minTempsDep[k] = (tmp[k + 1] < minTempsDep[k]) ? tmp[k + 1] : minTempsDep[k];
        }
    }

    for (int k = 0; k < nbMachines; k++) {
        minTempsArr[k] = 9999999;
    }
    minTempsArr[0] = 0;

    for (int i = 0; i < nbJob; i++) {
        for (int k = 0; k < nbMachines; k++) {
            tmp[k] = 0;
        }
        tmp[0] += tempsJob[i];
        for (int k = 1; k < nbMachines; k++) {
            tmp[k] = tmp[k - 1] + tempsJob[k * nbJob + i];
        }
        for (int k = 1; k < nbMachines; k++) {
            minTempsArr[k] = (tmp[k - 1] < minTempsArr[k]) ? tmp[k - 1] : minTempsArr[k];
        }
    }

    delete[]tmp;
} // bound_fsp_strong::remplirTempsArriverDepart

void
bound_fsp_strong::remplirTabJohnson()
{
    int cmpt = 0;

    // for all machine-pairs compute Johnson's sequence
    for (int i = 0; i < (nbMachines - 1); i++) {
        for (int j = i + 1; j < nbMachines; j++) {
            Johnson(tabJohnson + cmpt * nbJob, i, j, cmpt);
            cmpt++;
        }
    }
}

// ///////////////////////johnson.cpp

int
bound_fsp_strong::estInf(int i, int j)
{
    if (pluspetit[0][i] == pluspetit[0][j]) {
        if (pluspetit[0][i] == 1)
            return pluspetit[1][i] < pluspetit[1][j];

        return pluspetit[1][i] > pluspetit[1][j];
    }
    return pluspetit[0][i] < pluspetit[0][j];
}

int
bound_fsp_strong::estSup(int i, int j)
{
    if (pluspetit[0][i] == pluspetit[0][j]) {
        if (pluspetit[0][i] == 1)
            return pluspetit[1][i] > pluspetit[1][j];

        return pluspetit[1][i] < pluspetit[1][j];
    }
    return pluspetit[0][i] > pluspetit[0][j];
}

int
bound_fsp_strong::partionner(int * ordo, int deb, int fin)
{
    int d = deb - 1;
    int f = fin + 1;
    int mem, pivot = ordo[deb];

    do {
        do f--;
        while (estSup(ordo[f], pivot));
        do d++;
        while (estInf(ordo[d], pivot));

        if (d < f) {
            mem     = ordo[d];
            ordo[d] = ordo[f];
            ordo[f] = mem;
        }
    } while (d < f);
    return f;
}

void
bound_fsp_strong::quicksort(int * ordo, int deb, int fin)
{
    int k;

    if ((fin - deb) > 0) {
        k = partionner(ordo, deb, fin);
        quicksort(ordo, deb, k);
        quicksort(ordo, k + 1, fin);
    }
}

void
bound_fsp_strong::Johnson(int * ordo, int m1, int m2, int s)
{
    pluspetit[0] = (int *) malloc((nbJob) * sizeof(int));
    pluspetit[1] = (int *) malloc((nbJob) * sizeof(int));
    for (int i = 0; i < nbJob; i++) {
        ordo[i] = i;
        if (tempsJob[m1 * nbJob + i] < tempsJob[m2 * nbJob + i]) {
            pluspetit[0][i] = 1;
            pluspetit[1][i] = tempsJob[m1 * nbJob + i] + tempsLag[s * nbJob + i];
        } else {
            pluspetit[0][i] = 2;
            pluspetit[1][i] = tempsJob[m2 * nbJob + i] + tempsLag[s * nbJob + i];
        }
        // pluspetit[1] contains the smaller of JohPTM_i_m1 and JohPTM_i_m2
    }
    quicksort(ordo, 0, (nbJob - 1));

    free(pluspetit[0]);
    free(pluspetit[1]);
}

// ==============================
// COMPUTE BOUND
// ==============================
int
bound_fsp_strong::calculBorne(int minCmax)
{
    int valBorneInf[2];

    if (machinePairs == 3) {
        borneInfLearn(valBorneInf, minCmax, true);
    } else  {
        borneInfMakespan(valBorneInf, minCmax);
    }

    return valBorneInf[0];
}

// initial sequence
void
bound_fsp_strong::scheduleFront(int permutation[], int limite1, int limite2, int * idle)
{
    if (limite1 == -1) {
        memcpy(front, minTempsArr, nbMachines * sizeof(int));
        *idle = 0;
        return;
    }

    int job;
    for (int mm = 0; mm < nbMachines; mm++) {
        front[mm] = 0;
    }
    for (int j = 0; j <= limite1; j++) {
        job      = permutation[j];
        front[0] = front[0] + tempsJob[job];
        for (int m = 1; m < nbMachines; m++) {
            *idle   += std::max(0, front[m - 1] - front[m]);
            front[m] = std::max(front[m],
                front[m - 1]) + tempsJob[m * nbJob + job];
        }
    }
}

void
bound_fsp_strong::setFlags(int permutation[], int limite1, int limite2)
{
    memset(flag, 0, nbJob * sizeof(int));
    for (int j = 0; j <= limite1; j++) flag[permutation[j]] = -1;
    for (int j = limite2; j < nbJob; j++) flag[permutation[j]] = permutation[j] + 1;
}

// reverse problem...
void
bound_fsp_strong::scheduleBack(int permutation[], int limite2, int * idle)
{
    if (limite2 == nbJob) {
        memcpy(back, minTempsDep, nbMachines * sizeof(int));
        *idle = 0;
        return;
    }
    memset(back, 0, nbMachines * sizeof(int));

    int jobCour;
    for (int k = nbJob - 1; k >= limite2; k--) {
        jobCour = permutation[k];

        back[nbMachines - 1] += tempsJob[(nbMachines - 1) * nbJob + jobCour];
        for (int j = nbMachines - 2; j >= 0; j--) {
            *idle  += std::max(0, back[j + 1] - back[j]);
            back[j] = std::max(back[j], back[j + 1]) + tempsJob[j * nbJob + jobCour];
        }
    }
}

void
bound_fsp_strong::myswap(int * a, int * b)
{
    int tmp = *a;

    *a = *b;
    *b = tmp;
}

int
bound_fsp_strong::evalMakespan(int permutation[])
{
    int * tmp = new int[nbMachines];

    for (int m = 0; m < nbMachines; m++) tmp[m] = 0;

    for (int j = 0; j < nbJob; j++) {
        int job = permutation[j];
        tmp[0] += tempsJob[job];
        for (int m = 1; m < nbMachines; m++)
            tmp[m] = std::max(tmp[m], tmp[m - 1]) + tempsJob[m * nbJob + job];
    }
    int ret=tmp[nbMachines - 1];

    delete[] tmp;

    return ret;
}

void
bound_fsp_strong::bornes_calculer(int permutation[], const int limite1, const int limite2, int * couts, const int best)
{
    if (limite2 - limite1 <= 2) {
        //        printf("this happens\n");
        couts[0] = evalMakespan(permutation);
        //        couts[1]=couts[0];
    } else {
        setFlags(permutation, limite1, limite2);

        // set_nombres(limite1, limite2);
        // compute front
        couts[1] = 0;
        scheduleFront(permutation, limite1, limite2, &couts[1]);

        // compute tail
        scheduleBack(permutation, limite2, &couts[1]);

        couts[0] = calculBorne(best);
    }
}

int
bound_fsp_strong::evalSolution(int * permut)
{
    int * tmp = new int[nbMachines];

    for (int m = 0; m < nbMachines; m++) tmp[m] = 0;
    // *tardiness = 0;
    for (int j = 0; j < nbJob; j++) {
        int job = permut[j];
        tmp[0] = tmp[0] + tempsJob[job];
        for (int m = 1; m < nbMachines; m++) tmp[m] = std::max(tmp[m], tmp[m - 1]) + tempsJob[m * nbJob + job];
    }

    int ret = tmp[nbMachines - 1];

    delete[]tmp;
    return ret;
}

void
bound_fsp_strong::bornes_calculer(int * schedule, int limit1, int limit2)
{
    // bornes_calculer(p.permutation, p.limite1, p.limite2,p.couts,999999);
    // p.couts_somme=p.couts[0]+p.couts[1];
}

void
bound_fsp_strong::freeMem()
{
    free(flag);
    free(front);
    free(back);
    free(remain);
}
