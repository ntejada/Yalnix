int main(int argc, char *argv[]) {
	int rc;
	rc = Fork();
	if (rc == 0) {
		TtyPrintf(0, "alkfjiowafan vajenvionwevianeiuvnaweiovnawiovnauisdjfhuwenearubvhaeuvaeurvihaeuvajsvklajfoawin avjsdkfja wiomjas wajiofaj wfiojawefkajsfkwajefoja wejfi awjfajsiofawjf\n");
		rc = Fork();
		if(rc == 0) {
			TtyPrintf(0, "kaljfweoijfaiwjf aiwjfvas aifjaweiof awiofjaiosjf iaowjfiaowejf aiwefioa jfij awieofj awfjasdk jiavasmdivjawkfjaweiofjaio jisdf jawie fafsdfa sfawjifja *grandchild*\n");
		}
		else {
			TtyPrintf(0, "aklsjfaiowjf iasvkjn owjefewrfguawfjnw3 naiofnausfjnvguaiwenf na ajfasnv aijs hafsd nashf ioawe jfuasnbdva bfsndviansdvnbzxkjvnsadgfiuaw eaghasudfbgiuawehf ihewfui hwaebashdf *child*\n");
		}
	}
	else {
		TtyPrintf(0, "kaljfwoiajfdfkajwfio jasfmkasl jfaks fioawejf iaosjlks nvkalsmvklasdmvka nioaklsdj vajsif eiwaofj aksldgna sfnaginsrjgnasjfnunewaoifna uongeiurngakljgfasdvaio gfwaieugfais ngaawjief oiajf iasjfsjfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff *parent*\n");
	}
}
