import java.io.File;
import java.util.Random;
import processing.core.PApplet;
import processing.core.PImage;

public class MatchingGame {
  // Congratulations message
  private final static String CONGRA_MSG = "CONGRATULATIONS! YOU WON!";
  // Cards not matched message
  private final static String NOT_MATCHED = "CARDS NOT MATCHED. Try again!";
  // Cards matched message
  private final static String MATCHED = "CARDS MATCHED! Good Job!";
  // 2D-array which stores cards coordinates on the window display
  private final static float[][] CARDS_COORDINATES =
      new float[][] {{170, 170}, {324, 170}, {478, 170}, {632, 170}, {170, 324}, {324, 324},
          {478, 324}, {632, 324}, {170, 478}, {324, 478}, {478, 478}, {632, 478}};
  // Array that stores the card images filenames
  private final static String[] CARD_IMAGES_NAMES = new String[] {"apple.png", "ball.png",
      "peach.png", "redFlower.png", "shark.png", "yellowFlower.png"};
  private static PApplet processing; // PApplet object that represents
  // the graphic display window
  private static Card[] cards; // one dimensional array of cards
  private static PImage[] images; // array of images of the different cards
  private static Random randGen; // generator of random numbers
  private static Card selectedCard1; // First selected card
  private static Card selectedCard2; // Second selected card
  private static boolean winner; // boolean evaluated true if the game is won,
  // and false otherwise
  private static int matchedCardsCount; // number of cards matched so far
  // in one session of the game
  private static String message; // Displayed message to the display window

  /**
   * Defines the initial environment properties of this game as the program starts
   */
  public static void setup(PApplet processing) {
    int i;
    MatchingGame.processing = processing;
    // Set the color used for the background of the Processing window
    // processing.background(245, 255, 250); // Mint cream color
    images = new PImage[CARD_IMAGES_NAMES.length];
    // load i-th image file as PImage object and store its reference into images[i]
    for (i = 0; i < CARD_IMAGES_NAMES.length; i++) {
      images[i] = processing.loadImage("images" + File.separator + CARD_IMAGES_NAMES[i]);
    }

    initGame();
  }

  public static void initGame() {
    randGen = new Random(Utility.getSeed());
    selectedCard1 = null;
    selectedCard2 = null;
    matchedCardsCount = 0;
    winner = false;
    int i = 0;
    int j = 0;
    // 行
    int[] row = new int[CARDS_COORDINATES.length];
    float[] x = new float[2 * (CARD_IMAGES_NAMES.length)];
    float[] y = new float[2 * (CARD_IMAGES_NAMES.length)];
    // create the array cards for 3*4 = 12 cards
    cards = new Card[2 * (CARD_IMAGES_NAMES.length)];

    for (i = 0; i < CARDS_COORDINATES.length; i++) {
      row[i] = randGen.nextInt(CARDS_COORDINATES.length);
      for (j = 0; j < i; j++) {
        if (row[i] == row[j]) { // 和前面的重复了
          i--; // 如果重复了，先 -- 后 ++ 相当于下标不向前移动
          break;
        }
      }
    }


    for (i = 0; i < 2 * CARD_IMAGES_NAMES.length; i++) {
      x[i] = CARDS_COORDINATES[row[i]][0];
      y[i] = CARDS_COORDINATES[row[i]][1];
    }

    for (i = 0; i < CARD_IMAGES_NAMES.length; i++) {
      cards[i] = new Card(images[i], x[i], y[i]);
      cards[i + CARD_IMAGES_NAMES.length] =
          new Card(images[i], x[i + CARD_IMAGES_NAMES.length], y[i + CARD_IMAGES_NAMES.length]);
    }



  }

  /**
   * Callback method called each time the user presses a key
   */
  public static void keyPressed() {
    if (processing.key == 'n' || processing.key == 'N') {
      initGame();
    }
  }


  /**
   * Callback method draws continuously this application window display
   */
  public static void draw() {
    // Set the color used for the background of the Processing window
    processing.background(245, 255, 250); // Mint cream color
    for (int i = 0; i < 2 * CARD_IMAGES_NAMES.length; i++) {
      cards[i].draw();
    }
    displayMessage(message);
  }

  /**
   * Displays a given message to the display window
   * 
   * @param message to be displayed to the display window
   */
  public static void displayMessage(String message) {
    processing.fill(0);
    processing.textSize(20);
    processing.text(message, processing.width / 2, 50);
    processing.textSize(12);
  }

  /**
   * Checks whether the mouse is over a given Card
   * 
   * @return true if the mouse is over the storage list, false otherwise
   */
  public static boolean isMouseOver(Card card) {
    float xMouse = processing.mouseX;
    float yMouse = processing.mouseY;
    float height = card.getHeight();
    float width = card.getWidth();
    float xCard = card.getX();
    float yCard = card.getY();
    boolean mouseOver = false;
    if (xMouse >= (xCard - width / 2) && xMouse <= (xCard + width / 2)
        && yMouse >= (yCard - height / 2) && yMouse <= (yCard + height / 2)) {
      mouseOver = true;
    }

    return mouseOver;
  }

  /**
   * Callback method called each time the user presses the mouse
   */
  public static void mousePressed() {
   
    if(matchedCardsCount>=2) { 
      if(matchedCardsCount % 2 == 0) { 
        selectedCard1.deselect(); 
        selectedCard2.deselect();
        }
      if(matchingCards(selectedCard1, selectedCard2)==false) { 
        matchedCardsCount=matchedCardsCount-2;
        selectedCard1.setVisible(false); 
        selectedCard2.setVisible(false);
       }
      }
    
    for (int i = 0; i < 12; i++) {
      if (isMouseOver(cards[i])) { 
        matchedCardsCount++;
        if (cards[i].isVisible() == false) {
        cards[i].setVisible(true);
        cards[i].select();
        }else  matchedCardsCount--;
        if (matchedCardsCount % 2 == 1)
          selectedCard1 = cards[i];
        else
          selectedCard2 = cards[i];
      }
    }
    if(matchedCardsCount % 2 == 0) {
      if(matchingCards(selectedCard1, selectedCard2)) {message = MATCHED;}
    else {message = NOT_MATCHED;}}
    if(matchedCardsCount == 12)message = CONGRA_MSG;
  }

  /**
   * Checks whether two cards match or not
   * 
   * @param card1 reference to the first card
   * @param card2 reference to the second card
   * @return true if card1 and card2 image references are the same, false otherwise
   */
  public static boolean matchingCards(Card card1, Card card2) {
    if (card1.getImage().equals(card2.getImage())) {
      winner = true;
    } else {  
      winner = false;
    }
    return winner;
  }

  public static void main(String[] args) {

    Utility.runApplication();
  }

}
