# указать сколько посетителей + указать порт вахтера и его ip
# & чтобы запустить параллельно
for i in {1..100}
do
   ./visitors 127.0.0.1 5566 &
done
