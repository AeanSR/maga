#include <cstdio>
#include <cstring>
typedef union{
    unsigned key;
    struct{
        unsigned gcd : 1;
        unsigned rb_st : 2;
        unsigned bs_st : 2;
        unsigned er_rm : 4;
        unsigned bt_cd : 3;
        unsigned rage : 7;
    } state;
} state_t;

#define STATE_SPACE (1 << (1+2+2+4+3+7))
float v[STATE_SPACE];
float vnext[STATE_SPACE];
unsigned pi[STATE_SPACE];

enum action_set{
    wait,
    bt,
    rb,
    ws,
    NAA,
};
const float crit_rate = 0.3f;
const float discounting_factor = 0.995;
const float error_tolerance = 0.05;

int validate_state(state_t state){
    if (state.state.rb_st > 2) return 0;
    if (state.state.bs_st > 2) return 0;
    if (state.state.er_rm > 9) return 0;
    if (state.state.bt_cd > 5) return 0;
    if (state.state.rage > 120) return 0;
    return 1;
}

unsigned get_available_actions(state_t state){
    unsigned mask = 1; /* You can always wait. */
    if (state.state.gcd) return mask;
    if (!state.state.bt_cd) mask |= 2;
    if (state.state.rb_st && state.state.rage >= 10) mask |= 4;
    if (state.state.bs_st || state.state.rage >= 20) mask |= 8;
    return mask;
}

void value_update(state_t os){
    state_t ns;
    float this_action_reward;
    float instant_reward;
    float best_action_reward = -1;
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
            this_action_reward += discounting_factor * v[ns.key];
        break;
        case bt:
            /* none crit, none bs */
            instant_reward = 50.0 * (ns.state.er_rm ? 1.4 : 1.0);
            ns.state.gcd = 1;
            if(ns.state.er_rm) ns.state.er_rm --;
            ns.state.bt_cd = 5;
            if(ns.state.rage >= 106) ns.state.rage = 120;
            else ns.state.rage += 14;
            this_action_reward += 0.8f * (1.0 - crit_rate) * (instant_reward + discounting_factor * v[ns.key]);
            /* none crit, bs */
            t = ns.state.bs_st;
            ns.state.bs_st = 2;
            this_action_reward += 0.2f * (1.0 - crit_rate) * (instant_reward + discounting_factor * v[ns.key]);
            /* crit, none bs */
            ns.state.bs_st = t;
            instant_reward *= 2;
            if(ns.state.rage >= 110) ns.state.rage = 120;
            else ns.state.rage += 10;
            ns.state.er_rm = 9;
            this_action_reward += 0.8f * (crit_rate) * (instant_reward + discounting_factor * v[ns.key]);
            /* crit and bs */
            ns.state.bs_st = 2;
            this_action_reward += 0.2f * (crit_rate) * (instant_reward + discounting_factor * v[ns.key]);
        break;
        case rb:
            instant_reward = 325.0 * (ns.state.er_rm ? 1.4 : 1.0);
            instant_reward *= 1.0 + crit_rate;
            ns.state.gcd = 1;
            if(ns.state.er_rm) ns.state.er_rm --;
            if(ns.state.bt_cd) ns.state.bt_cd --;
            ns.state.rage -= 6;
            ns.state.rb_st --;
            this_action_reward += instant_reward + discounting_factor * v[ns.key];
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
            this_action_reward += instant_reward + discounting_factor * v[ns.key];
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

int main(){
    int converge = 0;
    int i = 1;
    while(!converge){
        printf("Value Iteration %d ...", i++);
        converge = 1;
        for(unsigned k = 0; k < STATE_SPACE; k++){
            state_t state;
            state.key = k;
            if(!validate_state(state)) continue;
            value_update(state);
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
}
