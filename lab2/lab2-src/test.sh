#pwcheck.sh
score=0
#git add $0 >> .local.git.out
#git commit -a -m "Lab2 commit" >> .local.git.out
#git push
#test1-1
echo qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq > passwordfile
./pwcheck.sh passwordfile > out1-1
line=$(head -n1 out1-1)
if [ "$line" == "Error: Password length invalid." ] ; then
	echo "Test1-1 passed";
	let score=score+1;
else
	echo "Test1-1 Failed";
	echo "Expected: Error: Password length invalid."
	echo "Actual: $line";
fi
#test1-2
echo abcd5 > passwordfile
./pwcheck.sh passwordfile > out1-2
line=$(head -n1 out1-2)
if [ "$line" == "Error: Password length invalid." ] ; then
	echo "Test1-2 passed";
	let score=score+1;
else
	echo "Test1-2 Failed";
	echo "Expected: Error: Password length invalid.";
	echo "Actual: $line";
fi
#test2-1
echo aBcDeF > passwordfile
./pwcheck.sh passwordfile > out2-1
line=$(head -n1 out2-1)
if [ "$line" == "Password Score: 11" ] ; then
	echo "Test2-1 passed";
	let score=score+2;
else 
	echo "Test2-1 Failed";
	echo "Expected: Password Score: 11";
	echo "Actual: $line";
fi
#test2-2
echo aBcDeFgHiJkLmNoPqRsTuVwXyZaBcDe > passwordfile
./pwcheck.sh passwordfile > out2-2
line=$(head -n1 out2-2)
if [ "$line" == "Password Score: 36" ] ; then
	echo "Test2-2 Passed";
	let score=score+2;
else
	echo "Test2-2 Failed";
	echo "Expected: Password Score: 36";
	echo "Actual: $line";
fi
#test3-1
echo aBc9F1 > passwordfile
./pwcheck.sh passwordfile > out3-1
line=$(head -n1 out3-1)
if [ "$line" == "Password Score: 16" ] ; then
	echo "Test3-1 Passed";
	let score=score+2;
else
	echo "Test3-1 Failed";
	echo "Expected: Password Score: 16";
	echo "Actual: $line";
fi
#test3-2
echo aBcDeFg94JkLmNo8qRsTuVwXyZaBcD2 > passwordfile
./pwcheck.sh passwordfile > out3-2
line=$(head -n1 out3-2)
if [ "$line" == "Password Score: 41" ] ; then
	echo "Test3-2 Passed";
	let score=score+2;
else
	echo "Test3-2 Failed";
	echo "Expected: Password Score 41";
	echo "Actual: $line";
fi
#test4-1
echo aBcDeF@ > passwordfile
./pwcheck.sh passwordfile > out4-1
line=$(head -n1 out4-1)
if [ "$line" == "Password Score: 17" ] ; then
	echo "Test4-1 Passed";
	let score=score+2;
else
	echo "Test4-1 Failed";
	echo "Expected: Password Score: 17";
	echo "Actual: $line";
fi
#test4-2
echo aBcDeF$ > passwordfile
./pwcheck.sh passwordfile > out4-2
line=$(head -n1 out4-2)
if [ "$line" == "Password Score: 17" ] ; then
	echo "Test4-2 Passed";
	let score=score+2;
else 
	echo "Test4-2 Failed";
	echo "Expected: Password Score: 17";
	echo "Actual: $line";
fi
#test5-1
echo AbCdEfGG > passwordfile
./pwcheck.sh passwordfile > out5-1
line=$(head -n1 out5-1)
if [ "$line" == "Password Score: 3" ] ; then
	echo "Test5-1 Passed";
	let score=score+2;
else
	echo "Test5-1 Failed";
	echo "Expected: Password Score: 3";
	echo "Actual: $line";
fi
#test5-2
echo aBcDeFgg > passwordfile
./pwcheck.sh passwordfile > out5-2
line=$(head -n1 out5-2)
if [ "$line" == "Password Score: 3" ] ; then
	echo "Test5-2 Passed";
	let score=score+2;
else
	echo "Test5-2 Failed";
	echo "Expected: Password Score: 3";
	echo "Actual: $line";
fi
#test6-1
echo aBcDeFgHiJkawfa > passwordfile
./pwcheck.sh passwordfile > out6-1
line=$(head -n1 out6-1)
if [ "$line" == "Password Score: 17" ] ; then
	echo "Test6-1 Passed";
	let score=score+2;
else
	echo "Test6-1 Failed";
	echo "Expected: Password Score: 17";
	echo "Actual: $line";
fi
#test6-2
echo abcdefghijkl > passwordfile
./pwcheck.sh passwordfile > out6-2
line=$(head -n1 out6-2)
if [ "$line" == "Password Score: 14" ] ; then
	echo "Test6-2 Passed";
	let score=score+2;
else
	echo "Test6-2 Failed";
	echo "Expected: Password Score:14";
	echo "Actual: $line";
fi
#test7-1
echo AbCdEfGhIjKAWFA > passwordfile
./pwcheck.sh passwordfile > out7-1
line=$(head -n1 out7-1)
if [ "$line" == "Password Score: 17" ] ; then
	echo "Test7-1 Passed";
	let score=score+2;
else
	echo "Test7-1 Failed";
	echo "Expected: Password Score: 17";
	echo "Actual: $line";
fi
#test7-2
echo ABCDEFGHIJKL > passwordfile
./pwcheck.sh passwordfile > out7-2
line=$(head -n1 out7-2)
if [ "$line" == "Password Score: 14" ] ; then
	echo "Test7-2 Passed";
	let score=score+2;
else
	echo "Test7-2 Failed";
	echo "Expected: Password Score: 14"
	echo "Actual: $line";
fi
#test8-1
echo 123456 > passwordfile
./pwcheck.sh passwordfile > out8-1
line=$(head -n1 out8-1)
if [ "$line" == "Password Score: 8" ] ; then
	echo "Test8-1 Passed";
	let score=score+2;
else
	echo "Test8-1 Failed";
	echo "Expected: Password Score: 8";
	echo "Actual: $line";
fi
#test8-2
echo ab12345f2456gD986f2e35a6f > passwordfile
./pwcheck.sh passwordfile > out8-2
line=$(head -n1 out8-2)
if [ "$line" == "Password Score: 32" ] ; then
	echo "Test8-2 Passed";
	let score=score+2;
else
	echo "Test8-2 Failed";
	echo "Expected: Password Score: 32";
	echo "Actual: $line";
fi
#test9-1
echo Exon#Mobi@Le21 > passwordfile
./pwcheck.sh passwordfile > out9-1
line=$(head -n1 out9-1)
if [ "$line" == "Password Score: 26" ] ; then
	echo "Test9-1 Passed";
	let score=score+5;
else
	echo "Test9-1 Failed";
	echo "Expected: Password Score: 26";
	echo "Actual: $line";
fi
#test9-2
echo 123456789abcdef@gDWSS@Aw4 > passwordfile
./pwcheck.sh passwordfile > out9-2
line=$(head -n1 out9-2)
if [ "$line" == "Password Score: 21" ] ; then
	echo "Test9-2 Passed";
	let score=score+5;
else
	echo "Test9-2 Failed";
	echo "Expected: Password Score: 21";
	echo "Actual: $line";
fi
echo "Total Score: $score of 40"
rm passwordfile
rm out1-1
rm out1-2
rm out2-1
rm out2-2
rm out3-1
rm out3-2
rm out4-1
rm out4-2
rm out5-1
rm out5-2
rm out6-1
rm out6-2
rm out7-1
rm out7-2
rm out8-1
rm out8-2
rm out9-1
rm out9-2
