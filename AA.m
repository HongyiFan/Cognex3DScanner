[X,Y] = meshgrid(1:200,1:200);

Point  = zeros
for i = 1:200
    for j = 1:200
        x = X(i,j);
        y = Y(i,j);
        
        I = undistort([x,y], K, k1,k2,k3,p1,p2)
        
    end
end