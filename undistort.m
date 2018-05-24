function I = undistort(P, K, k1,k2,k3,p1,p2)


%I = zeros(size(Idistorted));
%[i j] = find(~isnan(I));
invK = inv(K);
% Xp = the xyz vals of points on the z plane
Xp = [P 1]';
Xp(1:2) = Xp(1:2) - K(1:2,3);
%Xp = invK * Xp;
% Now we calculate how those points distort i.e forward map them through the distortion
r2 = Xp(1,:).^2+Xp(2,:).^2;
x = Xp(1,:);
y = Xp(2,:);

x = x.*(1+k1*r2 + k2*r2.^2);
y = y.*(1+k1*r2 + k2*r2.^2);
%A = K * [x y 1]'
I = [x y 1]'