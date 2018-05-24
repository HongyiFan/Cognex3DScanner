player = pcplayer([-300 300], [0 500], [731 1000]);

% rot = vrrotmat2vec(R1);
% stageTrans = makehgtform('axisrotate', rot(1:3), rot(4))*(makehgtform('translate', T1)');
% tform = affine3d(stageTrans);
% ptCloud = pointCloud(objPts);
% ptCloud = pctransform(ptCloud, tform);
% pcshow(ptCloud)

while isOpen(player)
    points = dlmread('3DPoints.txt');
    
    ptCloud = pointCloud(points);
    
    view(player,ptCloud);
end