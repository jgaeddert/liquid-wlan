% v is in memory

[rows cols] = size(v);

m = reshape(v,rows*cols,1);

for i=0:8:(rows*cols-1),
    msg = bin2dec(num2str(m([0:7] + i + 1))');

    printf('0x%.2x, ', msg);

    if ( mod((i/8)+1,5)==0 )
        printf("\n");
    end;
end;
printf("\n");
