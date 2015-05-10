#include <cstdio>
#include <cstring>
typedef union{
    unsigned key; /* Used as a array index. */
    struct{ /* State Space */
        unsigned gcd : 1;
        unsigned rb_st : 2;
        unsigned bs_st : 2;
        unsigned er_rm : 4;
        unsigned bt_cd : 3;
        unsigned rage : 7;
    } state;
} state_t;

#define STATE_SPACE (1 << (1+2+2+4+3+7))
double v[STATE_SPACE];     /* Uk(s) */
double vnext[STATE_SPACE]; /* Uk+1(s) */
unsigned pi[STATE_SPACE]; /* store the optimal policy. */

enum action_set{ /* Action Space */
    wait,
    bt,
    rb,
    ws,
    NAA,
};
const double crit_rate = 0.3; /* crit rate in game. */
const double error_tolerance = 0.001; /* error tolerance. */

/* Check if `state` is a valid state. */
int validate_state(state_t state){
    if (state.state.rb_st > 2) return 0;
    if (state.state.bs_st > 2) return 0;
    if (state.state.er_rm > 9) return 0;
    if (state.state.bt_cd > 5) return 0;
    if (state.state.rage > 120) return 0;
    return 1;
}

/* Return available action set As on `state`, as a bit mask. */
unsigned get_available_actions(state_t state){
    unsigned mask = 1; /* You can always wait. */
    if (state.state.gcd) return mask;
    if (!state.state.bt_cd) mask |= 2;
    if (state.state.rb_st && state.state.rage >= 10) mask |= 4;
    if (state.state.bs_st || state.state.rage >= 20) mask |= 8;
    return mask;
}

/* Bellman equation. */
void value_update(state_t os, int k){
    state_t ns;
    double this_action_reward;
    double instant_reward;
    double best_action_reward = -1;
    double discounting_factor_v = (double)k / (double)(k+1);
    double discounting_factor_i = 1.0 / (k+1);

    unsigned best_action = NAA;
    unsigned available_actions = get_available_actions(os);
    for(unsigned action = wait; action != NAA; action++){
        if (!(available_actions & (1 << action))) continue;
        this_action_reward = 0;
        instant_reward = 0;
        ns = os;
        unsigned t;

        switch(action){
        case wait:
            if(ns.state.gcd) ns.state.gcd --;
            if(ns.state.er_rm) ns.state.er_rm --;
            if(ns.state.bt_cd) ns.state.bt_cd --;
            if(ns.state.rage >= 116) ns.state.rage = 120;
            else ns.state.rage += 4;
            this_action_reward += discounting_factor_v * v[ns.key];
        break;
        case bt:
            /* none crit, none bs */
            instant_reward = 50.0 * (ns.state.er_rm ? 1.4 : 1.0);
            ns.state.gcd = 1;
            if(ns.state.er_rm) ns.state.er_rm --;
            ns.state.bt_cd = 5;
            if(ns.state.rage >= 106) ns.state.rage = 120;
            else ns.state.rage += 14;
            this_action_reward += 0.8 * (1.0 - crit_rate) * (discounting_factor_i * instant_reward + discounting_factor_v * v[ns.key]);
            /* none crit, bs */
            t = ns.state.bs_st;
            ns.state.bs_st = 2;
            this_action_reward += 0.2 * (1.0 - crit_rate) * (discounting_factor_i * instant_reward + discounting_factor_v * v[ns.key]);
            /* crit, none bs */
            ns.state.bs_st = t;
            instant_reward *= 2;
            if(ns.state.rage >= 110) ns.state.rage = 120;
            else ns.state.rage += 10;
            ns.state.rb_st = ns.state.rb_st == 2 ? 2 : ns.state.rb_st+1;
            ns.state.er_rm = 9;
            this_action_reward += 0.8 * (crit_rate) * (discounting_factor_i * instant_reward + discounting_factor_v * v[ns.key]);
            /* crit and bs */
            ns.state.bs_st = 2;
            this_action_reward += 0.2 * (crit_rate) * (discounting_factor_i * instant_reward + discounting_factor_v * v[ns.key]);
        break;
        case rb:
            instant_reward = 325.0 * (ns.state.er_rm ? 1.4 : 1.0);
            instant_reward *= 1.0 + crit_rate;
            ns.state.gcd = 1;
            if(ns.state.er_rm) ns.state.er_rm --;
            if(ns.state.bt_cd) ns.state.bt_cd --;
            ns.state.rage -= 6;
            ns.state.rb_st --;
            this_action_reward += discounting_factor_i * instant_reward + discounting_factor_v * v[ns.key];
        break;
        case ws:
            instant_reward = 234.375 * (ns.state.er_rm ? 1.4 : 1.0);
            instant_reward *= 1.0 + crit_rate;
            if(ns.state.er_rm) ns.state.er_rm --;
            if(ns.state.bt_cd) ns.state.bt_cd --;
            if(ns.state.bs_st){
                /* Bloodsurge */
                ns.state.rage = ns.state.rage >= 116 ? 120 : ns.state.rage + 4;
                ns.state.bs_st --;
            }else{
                /* Pure */
                ns.state.rage -= 16;
            }
            this_action_reward += discounting_factor_i * instant_reward + discounting_factor_v * v[ns.key];
        break;
        }

        if (this_action_reward > best_action_reward){
            best_action_reward = this_action_reward;
            best_action = action;
        }
    }
    pi[os.key] = best_action;
    vnext[os.key] = best_action_reward;
}

void demo();

int main(){
    int converge = 0;
    int k = 0;
    while(!converge){
        printf("Value Iteration %d ...", k++);
        converge = 1;
        for(unsigned i = 0; i < STATE_SPACE; i++){
            state_t state;
            state.key = i;
            if(!validate_state(state)) continue;
            value_update(state, k);
            if(vnext[state.key] - v[state.key] > error_tolerance || vnext[state.key] - v[state.key] < -error_tolerance) converge = 0;
        }
        printf("\b\b\b\n");
        memcpy(v, vnext, sizeof(v));
    }
    FILE* f = fopen("output.txt", "wb");
    const char* action_name[] = {
        "wait", "bt  ", "rb  ", "ws  ",
    };
    for(unsigned k = 0; k < STATE_SPACE; k++){
        state_t state;
        state.key = k;
        if(!validate_state(state)) continue;
        if(!state.state.gcd)
        fprintf(f, "RBST %d, BSST %d, ERRM %d, BTCD %d, RAGE %d - %s %.3f\r\n",
                state.state.rb_st, state.state.bs_st, state.state.er_rm, state.state.bt_cd, state.state.rage,
                action_name[pi[state.key]], v[state.key]);
    }

    demo();
}
