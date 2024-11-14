package edu.gmu.cs321;

import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import static javafx.geometry.HPos.RIGHT;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.PasswordField;
import javafx.scene.control.TextField;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;
import javafx.stage.Stage;

public class ApprovalScreen {
   private Scene ApprovalScreen;
   private Button emailButton = null;

   public ApprovalScreen() {
      GridPane grid = new GridPane();
      grid.setAlignment(Pos.CENTER);
      grid.setHgap(10);
      grid.setVgap(10);
      grid.setPadding(new Insets(25, 25, 25, 25));

      Text title = new Text("Approve Immigrant Data:");
      title.setFont(Font.font("Tahoma", FontWeight.NORMAL, 20));
      grid.add(title, 0, 0, 2, 1);

      Button lookupButton = new Button("Load Next Item");
      grid.add(lookupButton, 0, 1);

      TextField searchBox = new TextField();
      searchBox.setPromptText("Lookup by Green Card #...");
      Button searchButton = new Button("Search");
      HBox searchBoxContainer = new HBox(20);
      searchBoxContainer.setAlignment(Pos.CENTER_LEFT);
      searchBoxContainer.getChildren().addAll(searchBox, searchButton);
      grid.add(searchBoxContainer, 0, 3, 5, 1);

      Rectangle rectangle = new Rectangle(600, 450);
      rectangle.setFill(Color.GRAY);
      rectangle.setStroke(Color.BLACK);
      rectangle.setStrokeWidth(2);
      
      VBox labelBox = new VBox(5);
      labelBox.setAlignment(Pos.BASELINE_CENTER);
      Label boxFirstName = new Label("First Name:");
      TextField boxFN = new TextField();
      Label boxMiddleName = new Label("Middle Name:");
      TextField boxMN = new TextField();
      Label boxLastName = new Label("Last Name:");
      TextField boxLN = new TextField();
      Label boxAlienNumber = new Label("Alien Number:");
      TextField boxAN = new TextField();
      Label boxGCNumbr = new Label("Green Card Number:");
      TextField boxGCN = new TextField();
      Label boxDateOfBirth = new Label("Date of Birth:");
      TextField boxDOB = new TextField();
      Label boxAddress = new Label("Address:");
      TextField boxADD = new TextField();

      StackPane rectanglePane = new StackPane();

      HBox buttonBox = new HBox(10);
      buttonBox.setAlignment(Pos.BASELINE_CENTER);
      Button approveButton = new Button("Approve");
      Button rejectButton = new Button("Reject");

      Label messageLabel = new Label("Waiting for Approval");
      
      approveButton.setOnAction(e -> 
      {
         messageLabel.setText("Form approved! Confirmation email ready to be sent.");
         messageLabel.setTextFill(Color.GREEN);

         if (emailButton == null) {
            emailButton = new Button("Send email");
            emailButton.setOnAction(event -> messageLabel.setText("Email sent!"));
            messageLabel.setTextFill(Color.GREEN);
            buttonBox.getChildren().add(emailButton);
         }
      });
      rejectButton.setOnAction(e -> {
         messageLabel.setText("Form rejected! Sent back for additional review.");
         messageLabel.setTextFill(Color.RED);

         if (emailButton != null) {
            buttonBox.getChildren().remove(emailButton);
            emailButton = null;
         }
      });

      buttonBox.getChildren().addAll(approveButton, rejectButton, messageLabel);
      labelBox.getChildren().addAll(boxFirstName, boxFN, boxMiddleName, boxMN, boxLastName, boxLN, boxAlienNumber, boxAN, boxGCNumbr, boxGCN, boxDateOfBirth, boxDOB, boxAddress, boxADD, buttonBox);
      rectanglePane.getChildren().addAll(rectangle, labelBox);
      grid.add(rectanglePane, 0, 5);

      this.ApprovalScreen = new Scene(grid, 500, 475);
   }

   public Scene getApprovalScene() {
      return this.ApprovalScreen;
  }
}
