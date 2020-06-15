#include "../../common/headers/permutation.h"
#include "../../common/headers/subproblem.h"

#include "../headers/IG.h"

IG::IG(instance_abstract * inst)
{
    neh=new fastNEH();
    neh->set_instance(inst);
    neh->allocate();

    nhood=new fspnhood(inst);

    nbJob=nhood->m->nbJob;
    nbMachines=nhood->m->nbMachines;
    // nhood=new fspnhood(inst);

    perm=(int*)malloc(nbJob*sizeof(int));
    removed=(int*)malloc(nbJob*sizeof(int));

    int sum=0;
    for(int j=0;j<nbMachines;j++){
        for(int i=0;i<nbJob;i++){
            sum += nhood->m->PTM[j][i];
        }
    }
    // printf("%d %d %d\n",sum,nbJob,nbMachines);
    avgPT = (float)sum/(nbJob*nbMachines);
    acceptanceParameter=0.2;
	destructStrength=2;

    igiter=200;

	visitOrder=(int*)malloc(nbJob*sizeof(int));

	int start=nbJob/2;
	int ind=0;
	for(int i=start;i<nbJob;i++){
	// for(int i=nbJob-1;i>=start;i--){
		visitOrder[ind]=i;
		ind+=2;
	}
	ind=1;
	for(int i=start-1;i>=0;i--){
	// for(int i=0;i<start;i++){
		visitOrder[ind]=i;
		ind+=2;
	}
	// for(int i=0;i<nbJob;i++){
	// 	// visitOrder[i]=i;
	// 	printf("%d ",visitOrder[i]);
	// }
	// printf("\n");
}

IG::~IG()
{
    delete neh;

    free(perm);
    free(removed);
}

void IG::shuffle(int *array, int n)
{
    if (n > 1) {
        // size_t i;
	    for (int i = 0; i < n - 1; i++) {
			int j = helper::intRand(i, n-1);
	        // size_t j = i + drand48() / (RAND_MAX / (n - i) + 1);
	        int t = array[j];
	        array[j] = array[i];
	        array[i] = t;
	    }
    }
}

int IG::makespan(subproblem* s)
{
    return neh->evalMakespan(s->schedule, s->size);
}

//randomly removes k elements from perm and stores them in permout
void IG::destruction(int *perm, int *permOut, int k)
{
	//random permutation...
	// subproblem* sel=new subproblem(nbJob);
	// sel->shuffle(); //first k elements will be removed...


    // int r=helper::intRand(0, nbJob)+helper::intRand(0, (nbJob-1)/2);
    // // // //
    // int *sel=new int[k];
    // for(int i=0;i<k;i++){
    //     sel[i]=(r+i)%nbJob;
    //     // printf("%3d ",permOut[i]);
    // }

	//remove jobs from middle with higher probability
    int r=helper::intRand(k, nbJob);
    int *sel=new int[r];
    for(int i=0;i<r;i++)sel[i]=visitOrder[i];
    shuffle(sel, r);

    // int *sel=new int[nbJob];
    // for(int i=0;i<nbJob;i++)sel[i]=i;//visitOrder[i];
    // shuffle(sel, nbJob);



    int *flag=(int*)malloc(nbJob*sizeof(int));
    for(int i=0;i<nbJob;i++)flag[i]=1;

	//remove k jobs : copy to permOut
    for(int i=0;i<k;i++){
        permOut[i]=perm[sel[i]];
        flag[sel[i]]=0;
    }

	//prefix sum
    int *pref=(int*)calloc(nbJob,sizeof(int));
    for(int i=1;i<nbJob;i++){
        pref[i]=pref[i-1]+flag[i-1];
    }

    int *tmp=(int*)calloc(nbJob,sizeof(int));
    for(int i=0;i<nbJob;i++){
        if(flag[i]){
            tmp[pref[i]]=perm[i];
        }
    }

    for(int i=0;i<nbJob;i++){
        perm[i]=tmp[i];
    }

    free(flag);
    free(pref);
    free(tmp);
    //
    delete sel;
}

void IG::destruction(int *perm, int *permOut, int k, int a, int b)
{
    int* sel=(int*)malloc((b-a)*sizeof(int));
    for(int i=0;i<(b-a);i++){
        sel[i]=a+i;
    }

    shuffle(sel,(b-a));
    // for(int i=0;i<nbJob;i++){
    //     printf("%3d ",perm[i]);
    // }
    // printf("======\n");

    // permutation sel(nbJob);
    // sel.random();

    // sel.print();

    int *flag=(int*)malloc(nbJob*sizeof(int));
    for(int i=0;i<nbJob;i++)flag[i]=1;

    for(int i=0;i<k;i++){

        permOut[i]=perm[sel[i]];
        // printf("%d %d %d\n",i,sel[i],perm[sel[i]]);
        flag[sel[i]]=0;
    }

    int *pref=(int*)calloc(nbJob,sizeof(int));
    for(int i=1;i<nbJob;i++){
        pref[i]=pref[i-1]+flag[i-1];
    }

    int *tmp=(int*)calloc(nbJob,sizeof(int));
    for(int i=0;i<nbJob;i++){
        if(flag[i]){
            tmp[pref[i]]=perm[i];
        }
    }

    for(int i=0;i<nbJob;i++){
        perm[i]=tmp[i];
    }

    // for(int i=0;i<nbJob;i++){
    //     printf("%3d ",perm[i]);
    // }
    // printf("\n");
    // for(int i=0;i<nbJob;i++){
    //     printf("%3d ",permOut[i]);
    // }
    // printf("\n");

    free(sel);
    free(flag);
    free(pref);
    free(tmp);

    // delete sel;
}

void IG::perturbation(int *perm, int k, int a, int b)
{
	// int len=nbJob;
	// int cmax;

	// int kk=k;//helper::intRand(k-1, k+1);

	// adjacant swaps
	// for(int i=0;i<kk;i++)
	// {
	// 	int r1=helper::intRand(0,nbJob-2);
	// 	int rjob=nhood->m->remove(perm, len, r1);
	// 	nhood->m->insert(perm, len, r1+1, rjob);
	// }

    int* sel1=(int*)malloc((b-a)*sizeof(int));

    for(int i=0;i<(b-a);i++){
        sel1[i]=a+i;
    }
    shuffle(sel1,(b-a));

    int* sel2=(int*)malloc(k*sizeof(int));

    for(int i=0;i<k;i++){
        sel2[i]=sel1[i];
        sel1[i]=perm[sel1[i]];
    }

    shuffle(sel2,k);

    for(int i=0;i<k;i++){
        perm[sel2[i]]=sel1[i];
    }

    free(sel1);
    free(sel2);
}


void IG::construction(int *perm, int *permOut, int k)
{
    int cmax;
    // int bestpos;

    int len=nbJob-k;

    for(int j=0;j<k;j++){
        nhood->m->bestInsert(perm, len, permOut[j], cmax);
    }
}

void IG::blockConstruction(int *perm, int *permOut, int k)
{
    // int cmax;
    // int cost,bestpos;

    // int len=nbJob-k;

    for(int i=0;i<=nbJob-k;i++){
        //
    }
}




void IG::construction(int *perm, int *permOut, int k,int a, int b)
{
    // for(int i=0;i<nbJob-k;i++){
    //     printf("%3d ",perm[i]);
    // }
    // printf(" | ");
    // for(int i=0;i<k;i++){
    //     printf("%3d ",permOut[i]);
    // }
    //
    // printf("\n");

    int cost,bestpos;

    for(int j=0;j<k;j++){
        neh->computeHeads(perm,nbJob-k+j);
        neh->computeTails(perm,nbJob-k+j);
        neh->insertJob(perm, nbJob-k+j, permOut[j]);
        neh->findBestPos(cost,bestpos,nbJob-k+j,a,b-k+j);

        int tmp=permOut[j];
        for(int p=nbJob-k+j;p>bestpos;p--){
            perm[p]=perm[p-1];
        }
        perm[bestpos]=tmp;
    }

    // for(int i=0;i<nbJob;i++){
    //     printf("%3d ",perm[i]);
    // }
    // printf("\n");
    // for(int i=0;i<nbJob;i++){
    //     printf("%3d ",permOut[i]);
    // }
    // printf("\n");
}

bool IG::acceptance(int tempcost, int cost,float param)
{
    if(tempcost < cost){
        // printf("%d %d ",tempcost,cost);
        return true;
    }

    float earg=(float)cost-(float)tempcost;
    earg /= (param*avgPT/10.0);

	float r = helper::floatRand(0.0, 1.0);

    float prob=exp(earg);

    // printf("%d %d %1.10f %1.10f %1.10f \n",tempcost,cost,earg,r,prob);

    if(r<prob)return true;
    else return false;
}

int IG::runIG(subproblem* current)
{
    subproblem* temp=new subproblem(nbJob);
    subproblem* best=new subproblem(nbJob);
    subproblem* reduced=new subproblem(nbJob);

    int currentcost=0;
    int bestcost=0;

    *best=*current;
    bestcost=neh->evalMakespan(best->schedule, nbJob);
	currentcost=bestcost;

    int l1=current->limit1+1;
    int l2=current->limit2;

	// neh->runNEH(current->schedule,currentcost);
	// std::cout<<"cccc "<<currentcost<<std::endl;
	// currentcost=localSearchBRE(current->schedule);

    // return currentcost;
	int tempcost;
    int perturb=destructStrength;
    //
    for(int iter=0;iter<igiter;iter++){
		*temp=*current;
		// perturbation(temp->schedule, perturb, l1, l2);

        destruction(temp->schedule, reduced->schedule, perturb);
		tempcost=localSearchPartial(temp->schedule,nbJob-perturb);

        construction(temp->schedule, reduced->schedule, perturb,l1,l2);

		// tempcost=localSearchBRE(temp->schedule);
        // tempcost=localSearch(temp->schedule,l1,l2);

		int kmax=(int)sqrt(nbJob);
		tempcost=localSearchKI(temp->schedule,kmax);

		// printf(" aaa === %d %d %d\n",tempcost,bestcost,perturb);
		// temp->print();

        if(acceptance(tempcost, currentcost, acceptanceParameter)){
			currentcost=tempcost;
		    *current=*temp;
            perturb=destructStrength;
			// printf("%d\t",currentcost);
			// current->print();
        }else{
            perturb++;
            if(perturb>5)
                perturb=destructStrength;
        }
        if(tempcost < bestcost){
            bestcost=tempcost;
			*best=*temp;
        }
    }

    // bestcost=neh->evalMakespan(best->schedule, nbJob);
	// current->copy(best);
	// current->cost=bestcost;

    // best->print();
    // printf("%d\n",bestcost);

    delete temp;
    delete best;
    delete reduced;

    return bestcost;
}

int IG::runIG(subproblem* current,subproblem* guide)
{
    subproblem* temp=new subproblem(nbJob);
    subproblem* best=new subproblem(nbJob);
    subproblem* reduced=new subproblem(nbJob);

    int currentcost=0;
    int bestcost=0;

    // temp->copy(current);
	*best=*current;
    bestcost=neh->evalMakespan(best->schedule, nbJob);
	currentcost=bestcost;

	printf("curr %d\n",currentcost);
    // current->print();

    int l1=current->limit1+1;
    int l2=current->limit2;

	currentcost=ris(current,guide);
	// currentcost=localSearchBRE(current->schedule);

	int tempcost;
    int perturb=destructStrength;
    //
    for(int iter=0;iter<igiter;iter++){
		*temp=*current;
        // temp->copy(current);

		// perturbation(temp->schedule, perturb, l1, l2);

        destruction(temp->schedule, reduced->schedule, perturb,l1,l2);
		tempcost=localSearch(temp->schedule,l1,l2-perturb);

        construction(temp->schedule, reduced->schedule, perturb,l1,l2);

		// tempcost=localSearchBRE(temp->schedule);
        // tempcost=localSearch(temp->schedule,l1,l2);

		// int kmax=(int)sqrt(nbJob);

		tempcost=ris(temp,guide);
		// tempcost=localSearchKI(temp->schedule,kmax);

		// printf(" aaa === %d %d\n",tempcost,bestcost);
		// temp->print();

        if(acceptance(tempcost, currentcost, acceptanceParameter)){
			currentcost=tempcost;
			*current=*temp;

			// printf("%d\t",currentcost);
			// current->print();
        }
        if(tempcost < bestcost){
            bestcost=tempcost;
			*best=*temp;
        }
    }

    // bestcost=neh->evalMakespan(best->schedule, nbJob);
	*current=*best;

    // best->print();
    // printf("%d\n",bestcost);

    delete temp;
    delete best;
    delete reduced;

    return bestcost;
}

int
IG::localSearch(int *arr, int l1, int l2)
{
    // int *tmp1=(int*)malloc(nbJob*sizeof(int));
    int *tmp2=(int*)malloc(nbJob*sizeof(int));
    // memcpy(tmp1, arr, N*sizeof(int));

    int best=nhood->m->computeHeads(arr, nbJob);

    int c;

    // int depth = sqrt(nbJob);
    int depth = (l2-l1)/4;

    for(int k=0;k<10000;k++){
        memcpy(tmp2, arr, nbJob*sizeof(int));
        c=nhood->fastkImove(tmp2, depth,l1,l2);

        // if(acceptance(c, best, 0.01)){
        if(c<best){
            best=c;
            memcpy(arr, tmp2, nbJob*sizeof(int));
        }else{
            // printf("\t\t==== %d ===%3d\n",k,best);
            break;
        }
    }

    free(tmp2);

    return best;
}

int
IG::localSearchBRE(int *arr)
{
    int *tmp=(int*)malloc(nbJob*sizeof(int));
	// int *tmp2=(int*)malloc(nbJob*sizeof(int));

    int best=nhood->m->computeHeads(arr, nbJob);

    bool found;
    int c;

    for(int k=0;k<10000;k++){
        found=false;
        for(int i=0;i<nbJob;i++){
            memcpy(tmp, arr, nbJob*sizeof(int));
            c=nhood->fastBREmove(tmp, i);

            if(c<best){
                found=true;
                // memcpy(tmp2, tmp, nbJob*sizeof(int));
				best=c;
				memcpy(arr, tmp, nbJob*sizeof(int));
                break;
            }
        }
        if(!found){
            // printf("\t\t%3d\n",best);
            break;
        }
		// else{
            // memcpy(arr, tmp2, nbJob*sizeof(int));
        // }
    }

    free(tmp);
    // free(tmp2);

    return best;
}


int
IG::localSearchKI(int *arr,const int kmax)
{
    int *tmp=(int*)malloc(nbJob*sizeof(int));
    // int *tmp2=(int*)malloc(nbJob*sizeof(int));

    int best=nhood->m->computeHeads(arr, nbJob);

    bool found;
    int c;
	// printf("%d ",kmax);
	int i;

	//ls iterations ... 10000 = 'infinity' (likely getting trapped in local min much earlier)
    for(int k=0;k<10000;k++){
        found=false;
		//for all neighbors
        for(int j=0;j<nbJob;j++){
			i=visitOrder[j];

            memcpy(tmp, arr, nbJob*sizeof(int));
            c=nhood->kImove(tmp, i, kmax);//fastBREmove(tmp, i);

			//accept first improvement...
            if(c<best){
                found=true;
                best=c;
                // memcpy(tmp2, tmp, nbJob*sizeof(int));
                memcpy(arr, tmp, nbJob*sizeof(int));
                break; //accept first improvement...
            }
        }
        if(!found){
            // printf("\t\t%3d\n",best);
            break;
        }
		// else{
        //     memcpy(arr, tmp2, nbJob*sizeof(int));
		// }
    }

    free(tmp);
	// free(tmp2);

    return best;
}




int
IG::localSearchPartial(int *arr,const int N)
{
    int *tmp=(int*)malloc(nbJob*sizeof(int));
    // int *tmp2=(int*)malloc(nbJob*sizeof(int));

    int len=N;
    int best=nhood->m->computeHeads(arr, len);

    bool found;
    int c;
	// printf("%d ",kmax);
	// int i;

	//ls iterations ... 10000 = 'infinity' (likely getting trapped in local min much earlier)
    for(int k=0;k<10000;k++){
        found=false;
		//for all neighbors
        for(int j=0;j<len;j++){
			int i=j;//visitOrder[j];

            memcpy(tmp, arr, len*sizeof(int));

            int rjob=nhood->m->remove(tmp, len, i);
            nhood->m->bestInsert(tmp, len, rjob, c);

            // c=nhood->kImove(tmp, i, kmax);//fastBREmove(tmp, i);
			//accept first improvement...
            if(c<best){
                found=true;
                best=c;
                // memcpy(tmp2, tmp, nbJob*sizeof(int));
                memcpy(arr, tmp, len*sizeof(int));
                break; //accept first improvement...
            }
        }
        if(!found){
            // printf("\t\t%3d\n",best);
            break;
        }
		// else{
        //     memcpy(arr, tmp2, nbJob*sizeof(int));
		// }
    }

    free(tmp);
	// free(tmp2);

    return best;
}

//======================================================

// int
// IG::relinkInsert(subproblem* current, subproblem* guiding)
// {
//
// }



int
IG::vbih(subproblem* current, subproblem* guiding)
{
	subproblem* best=new subproblem(*current);
	subproblem* temp=new subproblem(*current);
    subproblem* reduced=new subproblem(nbJob);

    int bestcost=neh->evalMakespan(best->schedule, nbJob);
	int currentcost=bestcost;

	int min_bsize=2;
	int bsize=2;

	int tempcost;
	// int len=nbJob;

	int kmax=sqrt(nbJob);
	currentcost=localSearchKI(current->schedule,kmax);


    for(int k=1;k<igiter;k++){
	    min_bsize=1;
	    bsize=1;
        while(true)
        {
            *temp=*current;

            destruction(temp->schedule, reduced->schedule, bsize);

            // for(int i=0;i<bsize;i++){
            //     printf("%3d ",reduced->schedule[i]);
            // }
            // printf("\t|\t");

            // temp->print();
            // reduced->print();

            //localsearch on partial schedule
            localSearchPartial(temp->schedule,nbJob-bsize);
            // localSearchPartial(reduced->schedule,bsize);

            // temp->print();
            // reduced->print();

            //insert removed
            tempcost=nhood->m->bestBlockInsert(temp->schedule,nbJob-bsize,reduced->schedule,bsize);

            // len=nbJob;

            // temp->print();

            //local search on complete solution
    		// int kmax=(int)sqrt(nbJob);
    		// tempcost=localSearchKI(temp->schedule,kmax);

		    // tempcost=localSearchBRE(temp->schedule);
            // tempcost=localSearch(temp->schedule,0,nbJob);

            tempcost=ris(temp, guiding);

            // printf("%d %d %d\n",tempcost,currentcost,bsize);

            if(acceptance(tempcost, currentcost, acceptanceParameter)){
                bsize=min_bsize;
                currentcost=tempcost;
				*current=*temp;
                // printf("%d\t",currentcost);
                // current->print();
            }
            else{
                bsize++;
                // break;
            }
            if(tempcost < bestcost){
                *guiding = *temp;
                bestcost=tempcost;
				*best=*temp;
            }
            if(bsize>3)break;
        }
    }


    // printf("\n\n\n %d \n",bestcost);
    // best->print();

	delete best;
	delete temp;
    delete reduced;

    return bestcost;
}






int
IG::ris(subproblem* current, subproblem* guiding)
{
	subproblem* temp=new subproblem(*current);
    subproblem* star=new subproblem(*guiding);

    int best=nhood->m->computeHeads(temp->schedule, nbJob);

    int len=nbJob;
    int cmax=0;

    int i=0;
    int h=0;

    while(i<nbJob)
    {
        *temp=*current;

        int job1,rpos;
        // do{
            h = h%nbJob;
            // h = visitOrder[h];
            job1=star->schedule[h];//job at position h in guiding solution
            rpos=temp->locate(job1);//position of this job in current
            h++;
        // }while(rpos==h);

        if(rpos<0){printf("notfound\n");exit(-1);}

        int rjob=nhood->m->remove(temp->schedule, len, rpos);
        nhood->m->bestInsert(temp->schedule, len, rjob, cmax);

        if(cmax<best){
            *current=*temp;
            best=cmax;
            i=1;
        }else{
            i++;
        }
    }

	// *current=*temp;

    // printf("current %d\n",best);
	// current->print();

	delete star;
	delete temp;


    return best;
}
