package com.omok.unix_omok.model;

import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;

@Entity
public class OmokData {

    @Id
    private Long id;
    private String idBlack;
    private String idWhite;
    private String winner;
    private int playTime;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getIdBlack() {
        return idBlack;
    }

    public void setIdBlack(String idBlack) {
        this.idBlack = idBlack;
    }

    public String getIdWhite() {
        return idWhite;
    }

    public void setIdWhite(String idWhite) {
        this.idWhite = idWhite;
    }

    public String getWinner() {
        return winner;
    }

    public void setWinner(String winner) {
        this.winner = winner;
    }

    public int getPlayTime() {
        return playTime;
    }

    public void setPlayTime(int playTime) {
        this.playTime = playTime;
    }
}

