# ZDileptonAnalysis2017

mkdir 2017DATA_Zprime
cd 2017DATA_Zprime
cmsrel CMSSW_9_4_9
cd CMSSW_9_4_9/src
cmsenv
cd ..
mkdir oxbridge


cd /uscms_data/d3/username
mkdir oxbridge
cd oxbridge
wget http://www.hep.phy.cam.ac.uk/~lester/dtm662/mt2/Releases/oxbridgekinetics.tar.gz
tar -xzvf oxbridgekinetics.tar.gz
cd oxbridgekinetics-1.2
./configure --prefix=/uscms_data/d3/username/2017DATA_Zprime_2/CMSSW_9_4_9/oxbridge
(still inside oxbridgekinetics-1.2)
make
make install

then in
/uscms_data/d3/broozbah/2017DATA_Zprime_2/CMSSW_9_4_9/config/toolbox/slc6_amd64_gcc630/tools/selected
copied from you:
cp /uscms_data/d3/cihar29/newAnalysis/CMSSW_9_4_12/config/toolbox/slc6_amd64_gcc630/tools/selected/oxbridgekinetics-1.2.xml .

then in CMSSW_9_4_9
scram setup oxbridgekinetics-1.2
scram b
then in src
git clone https://github.com/broozbah/ZDileptonAnalysis2017
copied all related libraries