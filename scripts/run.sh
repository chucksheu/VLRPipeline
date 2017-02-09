cd FeatureExtract
./FeatureExtract -detect ORB /var/db/vlr/images /var/db/vlr/keypoints
./FeatureExtract -extract FREAK /var/db/vlr/images /var/db/vlr/keypoints /var/db/vlr/descriptors /var/db/vlr/freak_keypoints

ls -d -1 /var/db/vlr/descriptors/*.* > /var/db/vlr/in.list

cd ..
cd VocabLearn
./VocabLearn /var/db/vlr/in.list HKMAJ /var/db/vlr/tree.yaml.gz
cd ..
cd VocabBuildDB
./VocabBuildDB /var/db/vlr/in.list /var/db/vlr/tree.yaml.gz /var/db/vlr/inverted.yaml.gz TFIDF L2-norm

cd ..
cd FeatureExtract
./FeatureExtract -detect ORB /var/db/vlr/query_images /var/db/vlr/query_keypoints
./FeatureExtract -extract FREAK /var/db/vlr/query_images /var/db/vlr/query_keypoints /var/db/vlr/query_descriptors /var/db/vlr/query_freak_keypoints

for filename in `ls -d -1 /var/db/vlr/query_descriptors/*.*`; do echo $filename >>/var/db/vlr/query.list 0 0 600 800; done


cd ..
cd VocabMatch
./VocabMatch /var/db/vlr/tree.yaml.gz /var/db/vlr/inverted.yaml.gz /var/db/vlr/in.list /var/db/vlr/query.list /var/db/vlr/query_result -1 L2 COS  /var/db/vlr/query_result/result.html 0 /var/db/vlr/nn_index.bin