Ce programme est un premier projet de **Pricer d'options** pour moi. Ce code n'est pas parfait mais présente une **manière de concevoir intéressante**.



Vous pouvez compiler ce programme simplement comme ceci mais assurez vous d'avoir **gcc comme compilateur sur votre machine** :



g++ {path\_of\_file} -o monte\_carlo.exe





Et ensuite pour **executer** :



./monte\_carlo.exe





L'implémentation du multi-threading est une des principales caractéristiques de ce programme. Cela permet de meilleure performance. Un système de calcul de temps d'execution permet de mesurer justement ces performances. 
La première feature de ce programme est le fait d'utiliser le modèle de Monte-Carlo (utilisation de source aléatoire). Ce programme ne marche que pour des options européennes (le calcul se fait seulement à maturité T). 
