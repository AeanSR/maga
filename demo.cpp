#include <cstdlib>
#include <cstdio>
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
state_t s;
#define STATE_SPACE (1 << (1+2+2+4+3+7))

extern unsigned pi[STATE_SPACE];
enum action_set { /* Action Space */
    wait,
    bt,
    rb,
    ws,
    NAA,
};
const double crit_rate = 0.3; /* crit rate in game. */
const char* action_name[] = {
    "wait", "bt  ", "rb  ", "ws  ",
};

void do_action() {
    int action = pi[s.key];
    printf("do action %s\n", action_name[action]);
    switch( action ) {
    case wait:
        if( s.state.gcd ) s.state.gcd --;
        if( s.state.er_rm ) s.state.er_rm --;
        if( s.state.bt_cd ) s.state.bt_cd --;
        if( s.state.rage >= 116 ) s.state.rage = 120;
        else s.state.rage += 4;
        break;
    case bt: {
        double crit = ( double )rand() / RAND_MAX;
        double bs = ( double )rand() / RAND_MAX;
        if( crit >= crit_rate && bs >= 0.2 ) {
            /* none crit, none bs */
            s.state.gcd = 1;
            if( s.state.er_rm ) s.state.er_rm --;
            s.state.bt_cd = 5;
            if( s.state.rage >= 106 ) s.state.rage = 120;
            else s.state.rage += 14;
        } else if( crit >= crit_rate && bs < 0.2 ) {
            /* none crit, bs */
            s.state.gcd = 1;
            if( s.state.er_rm ) s.state.er_rm --;
            s.state.bt_cd = 5;
            if( s.state.rage >= 106 ) s.state.rage = 120;
            else s.state.rage += 14;
            s.state.bs_st = 2;
        } else  if( crit < crit_rate && bs >= 0.2 ) {
            /* crit, none bs */
            s.state.gcd = 1;
            if( s.state.er_rm ) s.state.er_rm --;
            s.state.bt_cd = 5;
            if( s.state.rage >= 106 ) s.state.rage = 120;
            else s.state.rage += 14;
            if( s.state.rage >= 110 ) s.state.rage = 120;
            else s.state.rage += 10;
            s.state.rb_st = s.state.rb_st == 2 ? 2 : s.state.rb_st+1;
            s.state.er_rm = 9;
        } else  if( crit < crit_rate && bs < 0.2 ) {
            /* crit and bs */
            s.state.gcd = 1;
            if( s.state.er_rm ) s.state.er_rm --;
            s.state.bt_cd = 5;
            if( s.state.rage >= 106 ) s.state.rage = 120;
            else s.state.rage += 14;
            if( s.state.rage >= 110 ) s.state.rage = 120;
            else s.state.rage += 10;
            s.state.rb_st = s.state.rb_st == 2 ? 2 : s.state.rb_st+1;
            s.state.er_rm = 9;
            s.state.bs_st = 2;
        }
    }
    break;
    case rb:
        s.state.gcd = 1;
        if( s.state.er_rm ) s.state.er_rm --;
        if( s.state.bt_cd ) s.state.bt_cd --;
        s.state.rage -= 6;
        s.state.rb_st --;
        break;
    case ws:
        if( s.state.er_rm ) s.state.er_rm --;
        if( s.state.bt_cd ) s.state.bt_cd --;
        if( s.state.bs_st ) {
            /* Bloodsurge */
            s.state.rage = s.state.rage >= 116 ? 120 : s.state.rage + 4;
            s.state.bs_st --;
        } else {
            /* Pure */
            s.state.rage -= 16;
        }
        break;
    }
}

void print_state(){
    const char* stars[] = {"  ", "* ", "**"};
    printf("RB: %s \tBS: %s\n", stars[s.state.rb_st], stars[s.state.bs_st]);
    printf("BTCD: [");
    for(int i = 0; i < s.state.bt_cd; i++) putchar('=');
    putchar('>');
    for(int i = s.state.bt_cd; i <= 6; i++) putchar(' ');
    printf("] %.1f\n", 0.75f * s.state.bt_cd);
    printf("ERRM: [");
    for(int i = 0; i < s.state.er_rm; i++) putchar('=');
    putchar('>');
    for(int i = s.state.er_rm; i <= 10; i++) putchar(' ');
    printf("] %.1f\n", 0.75f * s.state.er_rm);
    printf("RAGE: [");
    for(int i = 0; i < s.state.rage; i+=5) putchar('=');
    putchar('>');
    for(int i = s.state.rage/5; i <= 25; i++) putchar(' ');
    printf("] %d\n", s.state.rage);
}

void demo(){
    while(1){
        print_state();
        while(getchar()!='\n');
        do_action();
    }
}
