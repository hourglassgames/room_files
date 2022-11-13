const cards = document.querySelectorAll(".card");

let matched = 0;
let cardA, cardB;
let disableDeck = false;

function flipCard(e){
	let clickedCard = e.target;//gets user clicked card

	if(clickedCard !== cardA && !disableDeck){
		clickedCard.classList.add("flip");
		if(!cardA){
			return cardA = clickedCard;
		}//end of if

		cardB = clickedCard;
		disableDeck = true;
		let cardAImg = cardA.querySelector("img").src, cardBImg = cardB.querySelector("img").src;
		matchCards(cardAImg, cardBImg);
	}//end of if


}//end of flipCard

function matchCards(img1, img2){
	//if cards match
	if(img1 === img2){
		matched++;//count matches
		if(matched == 10){
		//HERE change to switch to next puzzle HERE
		window.location.replace('../tools.html')
			//  setTimeout(() => {
			//  	return shuffleCard();
			//  }, 800)

		}
		cardA.removeEventListener("click", flipCard);
		cardB.removeEventListener("click", flipCard);
		cardA = cardB = "";//sets both card values to blank
		return disableDeck = false;
	}//end of if

	//if cards don't match
	setTimeout(() => {
		//adding shake class to both cards after 400ms
		cardA.classList.add("shake");
		cardB.classList.add("shake");
	}, 400);

	setTimeout(() => {
		//remove shake and flip class from cards after 1.2 seconds
		cardA.classList.remove("shake", "flip");
		cardB.classList.remove("shake", "flip");
		cardA = cardB = "";//sets both card values to blank
		disableDeck = false;
		console.log(disableDeck);
	}, 800);

}//end of matchCards

function shuffleCard(){
	matched = 0;
	cardA = cardB = "";
	disableDeck = false;
	let arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
	arr.sort(() => Math.random() > 0.5 ? 1: -1);//sorts array randomly

	cards.forEach((card, index) => {
		card.classList.remove("flip");
		let imgTag = card.querySelector("img");
		imgTag.src = `./${arr[index]}.png`
		card.addEventListener("click", flipCard);
	});
}//end of shuffleCard

shuffleCard();

cards.forEach(card => {
	card.addEventListener("click", flipCard);
});