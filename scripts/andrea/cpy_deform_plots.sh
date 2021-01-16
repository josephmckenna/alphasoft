RUN=$1

cp ./AFTERampR${RUN}.pdf $AGRELEASE/AutoPlots/R${RUN}

cp ./PadAMPR${RUN}.pdf $AGRELEASE/AutoPlots/R${RUN}
cp ./PadAverageAmpR${RUN}.pdf $AGRELEASE/AutoPlots/R${RUN}
cp ./PadOccupancyR${RUN}_NoNorm.pdf $AGRELEASE/AutoPlots/R${RUN}

cp ./PadOverflowR${RUN}_NoNorm.pdf $AGRELEASE/AutoPlots/R${RUN}
cp ./AFTEROverflowOverOccupancyR${RUN}.pdf $AGRELEASE/AutoPlots/R${RUN}
