Q3ASM = q3asm
Q3CPP = q3cpp
Q3LCC = q3lcc
Q3RCC = q3rcc

CFLAGS = 

CG_OBJS = \
	../build/cgame/cg_main.asm \
	../build/cgame/bg_misc.asm \
	../build/cgame/bg_pmove.asm \
	../build/cgame/bg_slidemove.asm \
	../build/cgame/bg_lib.asm \
	../build/cgame/cg_consolecmds.asm \
	../build/cgame/cg_draw.asm \
	../build/cgame/cg_drawtools.asm \
	../build/cgame/cg_effects.asm \
	../build/cgame/cg_ents.asm \
	../build/cgame/cg_event.asm \
	../build/cgame/cg_info.asm \
	../build/cgame/cg_localents.asm \
	../build/cgame/cg_marks.asm \
	../build/cgame/cg_players.asm \
	../build/cgame/cg_playerstate.asm \
	../build/cgame/cg_predict.asm \
	../build/cgame/cg_scoreboard.asm \
	../build/cgame/cg_servercmds.asm \
	../build/cgame/cg_snapshot.asm \
	../build/cgame/cg_view.asm \
	../build/cgame/cg_weapons.asm \
	../build/cgame/q_math.asm \
	../build/cgame/q_shared.asm \


CG_SYSCALLS = code/cgame/cg_syscalls.asm

CG_QVM = ../vm/cgame.qvm

PAK = ../znudge_v1/zzznudge_v1.pk3

all : ${PAK}

${PAK} : ${CG_QVM}
	cd .. && zip -r znudge_src/${PAK} vm znudge_src

${CG_QVM} : ${CG_OBJS} ${CG_SYSCALLS}
	${Q3ASM} -o $@ ${CG_OBJS} ${CG_SYSCALLS}
	
../build/cgame/%.asm : code/cgame/%.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/cgame/bg_misc.asm : code/game/bg_misc.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/cgame/bg_move.asm : code/game/bg_move.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/cgame/bg_pmove.asm : code/game/bg_pmove.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/cgame/bg_slidemove.asm : code/game/bg_slidemove.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/cgame/bg_lib.asm : code/game/bg_lib.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/cgame/q_math.asm : code/game/q_math.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/cgame/q_shared.asm : code/game/q_shared.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

../build/qcommon/%.asm : code/qcommon/%.c
	${Q3LCC} ${CFLAGS} -DCGAME -o $@ $<

